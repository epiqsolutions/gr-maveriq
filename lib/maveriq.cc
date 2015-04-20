/* -*- c++ -*- */
/* 
 * Copyright 2013 Epiq Solutions.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "maveriq.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <netdb.h>
#include <string>
#include <math.h>
#include <stdexcept>
#include "srfs_interface.h"

namespace gr {
    namespace maveriq {

#define DEBUG_MAVERIQ 0
#define DEBUG(A)    if( DEBUG_MAVERIQ ) printf("=debug=> %s\n", A)

#define IQ_HEADER_SIZE (sizeof(srfs::BINARY_IQ))
#define BINARY_HEADER_SIZE (sizeof(srfs::BINARY))

// TODO: add this as a configuration parameter to the block
// Note: there is a tradoff here between latency and performance
// This value accounts for the frequency in which sample data is
// provided.  The higher the packet size, the greater latency but 
// improved performance
#define PKT_SIZE (40960)
#define NUM_SAMPLES (PKT_SIZE/sizeof(uint32_t))


#define NUM_RECV_ATTEMPTS (3)

#define FREQUENCY_MIN  47000000ULL
#define FREQUENCY_MAX 6000000000ULL
#define FREQUENCY_RESOLUTION 1

#define SAMPLE_RATE_MIN   233000
#define SAMPLE_RATE_MAX 40000000
#define SAMPLE_RATE_RESOLUTION 1

#define BANDWIDTH_MIN   233000
#define BANDWIDTH_MAX 40000000
#define BANDWIDTH_RESOLUTION 1

#define RX_GAIN_MIN        0
#define RX_GAIN_MAX       76
#define RX_GAIN_RESOLUTION 1

#define STEP_ATTEN_MIN 0
#define STEP_ATTEN_MAX 31
#define STEP_ATTEN_RESOLUTION 1

maveriq::maveriq(const char* addr, unsigned short port)
{
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;

    d_server = gethostbyname( addr );

    if ((d_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	throw std::runtime_error("unable to create socket");
    }

    if( setsockopt( d_sock, SOL_SOCKET, SO_RCVTIMEO, 
		    (char*)(&tv), sizeof(struct timeval)) != 0 )
    {
	throw std::runtime_error("unable to set socket options");
    }

    memset( &d_server_addr, 0, sizeof(d_server_addr) );
    d_server_addr.sin_family = AF_INET;
    d_server_addr.sin_port = htons(port);
    d_server_addr.sin_addr.s_addr = INADDR_ANY;
    memcpy( &d_server_addr.sin_addr.s_addr,
            d_server->h_addr,
            d_server->h_length );

    if ( 0 != connect( d_sock,
                       (struct sockaddr *)&d_server_addr,
                       sizeof( d_server_addr ) ) ) {
	throw std::runtime_error("unable to connect to maveriq");
    }

    d_addr_len = sizeof(struct sockaddr);

    // set default values
    d_rx_freq = FREQUENCY_MIN;
    d_rx_sample_rate = SAMPLE_RATE_MIN;
    d_rx_bandwidth = BANDWIDTH_MIN;
    d_rx_front_lna_status = STATUS_ENABLED;
    d_rx_second_lna_status = STATUS_ENABLED;
    d_rx_gain = 50;
    d_rx_step_atten = STEP_ATTEN_MIN;
    d_rx_gain_mode = GAIN_MODE_MANUAL;
    d_srfs_src_status = STATUS_DISABLED;

    d_state = STATE_STOPPED;

    init_srfs_params();

    // Get SRFS spun up
    open_srfs();
}

maveriq::~maveriq()
{
    maveriq::stop();
    close_srfs();
    close(d_sock);
}

void
maveriq::init_srfs_params(void)
{
    // frequency
    add_srfs_param( "A1:frequency",
		    srfs::SRFS_UINT64,
		    (void*)(&d_rx_freq),
		    FREQUENCY_MIN,
		    FREQUENCY_MAX,
		    FREQUENCY_RESOLUTION,
		    NULL );

    // sample rate
    add_srfs_param( "A1:sample_rate",
		    srfs::SRFS_UINT32,
		    (void*)(&d_rx_sample_rate),
		    SAMPLE_RATE_MIN,
		    SAMPLE_RATE_MAX,
		    SAMPLE_RATE_RESOLUTION,
		    NULL );

    // bandwidth
    add_srfs_param( "A1:bandwidth",
		    srfs::SRFS_UINT32,
		    (void*)(&d_rx_bandwidth),
		    BANDWIDTH_MIN,
		    BANDWIDTH_MAX,
		    BANDWIDTH_RESOLUTION,
		    NULL );

    // actual bandwidth, this parameter cannot be configured but represents
    // the actual bandwidth setting
    add_srfs_param( "A1:actual_bandwidth",
		    srfs::SRFS_UINT32_ACTUAL,
		    (void*)(&d_rx_actual_bandwidth),
		    0,
		    0,
		    0,
		    NULL );

    // front lna
    add_srfs_param( "A1:lna1",
		    srfs::SRFS_ENUM,
		    (void*)(&d_rx_front_lna_status),
		    0,
		    STATUS_INVALID,
		    1, 
		    status_str );

    // second lna
    add_srfs_param( "A1:lna2",
		    srfs::SRFS_ENUM,
		    (void*)(&d_rx_second_lna_status),
		    0,
		    STATUS_INVALID,
		    1, 
		    status_str );		    

    // rx gain
    add_srfs_param( "A1:rx_gain",
		    srfs::SRFS_UINT8,
		    (void*)(&d_rx_gain),
		    RX_GAIN_MIN,
		    RX_GAIN_MAX,
		    RX_GAIN_RESOLUTION,
		    NULL );

    // step atten
    add_srfs_param( "A1:step_atten",
		    srfs::SRFS_UINT8,
		    (void*)(&d_rx_step_atten),
		    STEP_ATTEN_MIN,
		    STEP_ATTEN_MAX,
		    STEP_ATTEN_RESOLUTION,
		    NULL );

    // gain mode
    add_srfs_param( "A1:gain_mode",
		    srfs::SRFS_ENUM,
		    (void*)(&d_rx_gain_mode),
		    0, 
		    GAIN_MODE_INVALID,
		    1,
		    gain_mode_str );
}

void 
maveriq::add_srfs_param( const std::string token,
			 srfs::SRFS_DATATYPES data_type,
			 void *p_value,
			 int64_t min_value,
			 int64_t max_value,
			 float resolution,
			 const std::string *p_strings )
{
    srfs::srfs_param_t param;

    param.data_type = data_type;
    param.p_value = p_value;
    param.min_value = min_value;
    param.max_value = max_value;
    param.resolution = resolution;
    param.p_strings = p_strings;

    maveriq_params[token] = param;
}

void
maveriq::set_param( const std::string token, void *pValue )
{
    param_map::iterator iter;

    if( d_state == STATE_STARTED ) {
	iter = maveriq_params.find(token);
	if( iter != maveriq_params.end() ) {
	    if( srfs::set_param(&(iter->second), pValue) ) {
		config_src();
	    }
	}
	else {
	    throw std::invalid_argument("unknown parameter specified");
	}
    }
    else {
	throw std::runtime_error("no maveriq connection when trying to set param");
    }
}

uint64_t
maveriq::set_center_freq(uint64_t rx_freq)
{
    set_param( "A1:frequency", &rx_freq );
    return d_rx_freq;
}

uint64_t
maveriq::center_freq(void)
{
    return d_rx_freq;
}

uint32_t
maveriq::set_sample_rate(uint32_t rx_sample_rate)
{
    set_param("A1:sample_rate", &rx_sample_rate);
    return d_rx_sample_rate;
}

uint32_t
maveriq::sample_rate(void)
{
    return d_rx_sample_rate;
}

uint32_t
maveriq::set_bandwidth(uint32_t rx_bandwidth)
{
    set_param("A1:bandwidth", &rx_bandwidth);
    return d_rx_bandwidth;
}

uint32_t
maveriq::bandwidth(void)
{
    return d_rx_actual_bandwidth;
}

STATUS
maveriq::set_front_lna( STATUS rx_lna_status )
{
    set_param("A1:lna1", &rx_lna_status);
    return d_rx_front_lna_status;
}

STATUS
maveriq::front_lna( void )
{
    return d_rx_front_lna_status;
}

STATUS 
maveriq::set_second_lna(STATUS enable)
{
    set_param("A1:lna2", &enable);
    return d_rx_second_lna_status;
}

STATUS 
maveriq::second_lna(void)
{
    return d_rx_second_lna_status;
}
	    
uint8_t 
maveriq::set_rx_gain(uint8_t gain)
{
    set_param("A1:rx_gain", &gain);
    return d_rx_gain;
}

uint8_t 
maveriq::rx_gain(void)
{
    return d_rx_gain;
}
	    
uint8_t 
maveriq::set_step_attenuator(uint8_t step_atten)
{
    set_param("A1:step_atten", &step_atten);
    return d_rx_step_atten;
}

uint8_t 
maveriq::step_attenuator(void)
{
    return d_rx_step_atten;
}
	    
GAIN_MODE 
maveriq::set_rx_gain_mode(GAIN_MODE mode)
{
    set_param("A1:gain_mode", &mode);
    return d_rx_gain_mode;
}

GAIN_MODE 
maveriq::rx_gain_mode(void)
{
    return d_rx_gain_mode;
}

void 
maveriq::open_srfs()
{
    char cmd[1024];
    char rcv[1024];

    std::string str1;
    std::string str2;
    std::string str3;

    // reset to the default state
    snprintf(cmd, 1024, "reset!\n");
    send_msg( cmd );
    receive_msg( rcv, 1024 ); // clear out rcv buf

    // subscribe to MAVERIQ-RX
    snprintf(cmd, 1024, "subscribe! block MAVERIQ-RX\n");
    send_msg( cmd );
    receive_msg( rcv, 1024 );
    
    // parse out source port
    str1 = std::string( rcv );
    str2 = str1.substr( str1.find(":") + 1 ); // Move one past :
    str3 = str2.substr( 0, str2.find(" ") ); // src_port is between : and space
    d_src_port = atoi( str3.c_str() );

    // subscribe to IQ
    snprintf(cmd, 1024, "subscribe! block IQ\n"); 
    send_msg( cmd );
    receive_msg( rcv, 1024 );
    
    // parse out IQ port
    str1 = std::string( rcv );
    str2 = str1.substr( str1.find(":") + 1 ); // Move one past :
    str3 = str2.substr( 0, str2.find(" ") ); // iq_port is between : and space
    d_iq_port = atoi( str3.c_str() );

    // configure IQ to MAVERIQ-RX
    snprintf(cmd, 1024, "config! block IQ:%d input MAVERIQ-RX:%d:A1 block_send %d\n", 
	     d_iq_port, d_src_port, (int32_t)(NUM_SAMPLES));
    send_msg( cmd );
    receive_msg( rcv, 1024 );
    
    snprintf(cmd, 1024, 
	     "config! block IQ:%d duty_cycle continuous\n", 
	     d_iq_port);

    send_msg( cmd );
    receive_msg( rcv, 1024 );

    d_state = STATE_STARTED;
    d_srfs_src_status = STATUS_ENABLED;
}

void 
maveriq::start()
{
    char cmd[1024];
    char rcv[1024];

    if( DEBUG_MAVERIQ ) { 
	printf("starting\n");
    }

    // make sure it's configured
    config_src();

    //###################################
    //#     Now Open new IQ Port        #
    //###################################
    d_iq_sock = socket( AF_INET, SOCK_STREAM, 0 );

    memset( &d_iq_server_addr, 0, sizeof(d_iq_server_addr) );
    d_iq_server_addr.sin_family = AF_INET;
    d_iq_server_addr.sin_port = htons( d_iq_port );
    memcpy( &d_iq_server_addr.sin_addr.s_addr,
        d_server->h_addr,
        d_server->h_length );

    if ( 0 != connect( d_iq_sock,
                       (struct sockaddr *)&d_iq_server_addr,
                       sizeof(d_iq_server_addr) ) ) {
	throw std::runtime_error("unable to connect to IQ socket");
    }

    // enable the IQ data
    snprintf( cmd, 1024,
	     "config! block IQ:%d status enabled\n",
	      d_iq_port );
    send_msg(cmd);
    receive_msg(rcv, 1024);

    first = true;
    
    if( DEBUG_MAVERIQ ) {
	printf("Successfully opened both ports\n");
    }
}

void 
maveriq::close_srfs()
{
    char cmd[1024];
    char rcv[1024];

    if ( d_state == STATE_STARTED ) {

	// unsubscribe from the RX block
        snprintf(cmd, 1024, "unsubscribe! block MAVERIQ-RX:%d\n", d_src_port);
        send_msg( cmd );
        receive_msg( rcv, 1024 ); // clear out rcv buf
	// unsubscribe from IQ block
        snprintf(cmd, 1024, "unsubscribe! block IQ:%d\n", d_iq_port);
        send_msg( cmd );
        receive_msg( rcv, 1024 ); // clear out rcv buf
    }
}

void 
maveriq::stop()
{
    char cmd[1024];
    char rcv[1024];
    char data[PKT_SIZE];
    ssize_t num_bytes = PKT_SIZE;
    uint8_t count = 0;

    if( DEBUG_MAVERIQ ) {
	printf("stopping\n");
    }

    if( d_state == STATE_STARTED ) {
	snprintf( cmd, 1024,
		 "config! block IQ:%d status disabled\n",
		 d_iq_port );
	send_msg( cmd );
	receive_msg( rcv, 1024 );

	// make sure that the IQ socket is flushed out
	while( count < 2 ) {
	    num_bytes = recv( d_iq_sock,
			      data,
			      PKT_SIZE,
			      MSG_DONTWAIT );
	    if( num_bytes != -1 ) {
		count = 0;
	    }
	    else {
		count++;
		usleep(100*1000);
	    }
	}
	// shutdown the IQ socket
	shutdown(d_iq_port, 2);
    }
    d_state = STATE_STOPPED;
}

void 
maveriq::send_msg( char * cmd )
{
    int flags = 0;
    if (DEBUG_MAVERIQ) {
	printf("sending: %s\n", cmd);
    }

    sendto( d_sock, cmd, strlen(cmd)+1, flags,
	    (const sockaddr*)&d_server_addr, d_addr_len);
    // Make sure to not send messages too fast
    usleep(200000);
}

int 
maveriq::receive_msg( char * rcv, int size )
{
    int flags = 0;
    int num_bytes;

    // TODO: add timeout, fail and check for fail

    memset( rcv, 0, size );
    num_bytes = recvfrom( d_sock, rcv, size, flags,
	                    (struct sockaddr *)&d_server_addr, &d_addr_len);
    if( num_bytes <= 0 )
    {
	throw std::runtime_error("failed to receive response");
    }

    if (DEBUG_MAVERIQ) {
	printf("receive: %s\n", rcv);
    }

    return num_bytes;
}    
    
void 
maveriq::config_src()
{
    char cmd[1024];
    char rcv[10000];

    char *pParam;
    char *pValue;
    
    int index=0;

    param_map::iterator iter;

    index = snprintf(cmd, 1024, "config! block MAVERIQ-RX:%d", d_src_port);
    // configure all of the parameters
    for( iter=maveriq_params.begin(); 
	 iter != maveriq_params.end(); 
	 iter++ ) {
        
        if( iter->second.data_type != srfs::SRFS_UINT32_ACTUAL ) {
            index += snprintf( &cmd[index], 1024-index, " %s ", (iter->first).c_str() );
        }

	// format the parameters based on data_type
	switch( iter->second.data_type ) {
	    case srfs::SRFS_UINT64:
		index += snprintf( &cmd[index], 1024-index, "%lu", 
				   (*(uint64_t*)(iter->second.p_value)) );
		break;

	    case srfs::SRFS_UINT32:
		index += snprintf( &cmd[index], 1024-index, "%u", 
				   (*(uint32_t*)(iter->second.p_value)) );
		break;

	    case srfs::SRFS_UINT16:
		index += snprintf( &cmd[index], 1024-index, "%hu", 
				   (*(uint16_t*)(iter->second.p_value)) );
		break;

	    case srfs::SRFS_UINT8:
		index += snprintf( &cmd[index], 1024-index, "%hhu", 
				   (*(uint8_t*)(iter->second.p_value)) );
		break;

	    case srfs::SRFS_FLOAT:
		index += snprintf( &cmd[index], 1024-index, "%f", 
				   (*(float*)(iter->second.p_value)) );
		break;

	    case srfs::SRFS_ENUM:
		index += snprintf( &cmd[index], 1024-index, "%s", 
				   (iter->second.p_strings[(*(int*)(iter->second.p_value))]).c_str() );
		break;

            case srfs::SRFS_UINT32_ACTUAL:
                // this parameter is only returned as an actual parameter, so don't 
                // allow it to be set
                break;
	}
    }
    index += snprintf( &cmd[index], 1024-index, " A1:status %s\n",
		       status_str[d_srfs_src_status].c_str());
    send_msg( cmd );
    receive_msg( rcv, 10000 );

    // parse the response, update the returned parameters
    pParam = strtok( rcv, " " );
    while( pParam != NULL )	{
	iter = maveriq_params.find(pParam);
	if( iter != maveriq_params.end() ) {
	    pParam = strtok( NULL, " " );
	    update_param( &(iter->second), (const char*)(pParam) );
	}
	else if( strncmp(pParam, "NOK", 4) == 0 ) {
	    throw std::invalid_argument("unexpected error with config_src");
	}
	// get the next parameter
	pParam = strtok( NULL, " " );
    } // end parsing
}

int 
maveriq::read(char* buf, int size)
{
    static char data[PKT_SIZE];
    static uint32_t dataIndex=PKT_SIZE;  // initialize to max, forcing data retrieval
    static uint64_t old_timestamp;

    uint64_t timestamp_diff;
    ssize_t num_bytes;
    char header[IQ_HEADER_SIZE];
    bool bMoreData = false;
    int num_bytes_processed = 0;
    int recv_result;
    int count = 0;

    // see if this is the first block we're receiving
    if( first ) {
	// reset the index back to max to force more data retrieval
	dataIndex = PKT_SIZE;
	first = false;
    }

    int16_t *tmp = (int16_t*)(&data[dataIndex]);
    int16_t *tmpBuf = (int16_t*)(buf);
    uint32_t i=0;
    for( i=0; (i<(size/2)) && (dataIndex < PKT_SIZE); i++ ) {
	tmpBuf[i] = be16toh( tmp[i] );
	num_bytes_processed += 2;
	dataIndex += 2;
    } 

    // see if we've ran out of buffer space
    if( dataIndex >= PKT_SIZE ) {
	bMoreData = true;
    }
    
    // need to provide more data, try to retrieve it
    if( bMoreData ) {
	num_bytes = 0;

	// try to receive just the IQ header
	while( num_bytes < IQ_HEADER_SIZE ) {
	    // try to get data but don't block
	    recv_result = recv( d_iq_sock, 
				header + num_bytes,
				IQ_HEADER_SIZE - num_bytes,
				MSG_DONTWAIT );
	    if( recv_result < 0 ) {
		count++;
		if( count >= NUM_RECV_ATTEMPTS ) {
		    num_bytes_processed = -1;
		    first = true;
		    goto end_recv;
		}
		usleep(10*1000);
	    }
	    else
	    {
		count = 0;
		num_bytes += recv_result;
	    }
	}
	srfs::BINARY_IQ* binary_iq = (srfs::BINARY_IQ*)(header);
	// convert to host format
	srfs::BINARY_IQ_to_host( binary_iq );
	// get the length of the payload from the header
	uint32_t length = binary_iq->binary.length - IQ_HEADER_SIZE + BINARY_HEADER_SIZE;

	num_bytes = 0;
	dataIndex = 0;
	count = 0;
	// receive the payload
	while( num_bytes < length ) {
	    // read in the data but don't block
	    recv_result = recv( d_iq_sock,
				data + num_bytes,
				length - num_bytes,
				MSG_DONTWAIT );
	    if( recv_result < 0 ) {
		count++;
		if( count >= NUM_RECV_ATTEMPTS ) {
		    num_bytes_processed = -1;
		    first = true;
		    goto end_recv;
		}
		usleep(10*1000);
	    }
	    else {
		count = 0;
		num_bytes += recv_result;
	    }
	}
	// Check for dropped samples
	timestamp_diff = binary_iq->timestamp - old_timestamp;
	if ( timestamp_diff > (NUM_SAMPLES) )
	{
	    printf("Dropped %lu samples\n", timestamp_diff-NUM_SAMPLES );
	}
	old_timestamp = binary_iq->timestamp;
    }

end_recv:
    return num_bytes_processed;
}

    } // namespace maveriq
} // namespace gr
