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

#ifndef MAVERIQ_H
#define MAVERIQ_H

#include <maveriq/maveriq_defs.h>
#include <boost/scoped_ptr.hpp>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdint.h>
#include <string>
#include <netdb.h>
#include <string>
#include <map>

#include "srfs_interface.h"

namespace gr {
    namespace maveriq {
	/*! String representation of STATUS enum */
	const std::string status_str[STATUS_INVALID] = {
	    "enabled",
	    "disabled"
	};

	/*! String representation of GAIN_MODE enum */
	const std::string gain_mode_str[GAIN_MODE_INVALID] = {
	    "manual"
	};

	/*! Implements interface to maveriq */
	class maveriq {
	    
	public:
	    
	    /*!< state of maveriq connection */
	    enum state {
		STATE_STOPPED, STATE_STARTED,
	    };
	    
	    maveriq(const char* ip_addr, unsigned short port);
	    ~maveriq();


	    void start();
	    void stop();

	    // read the data from maveriq
	    int read(char* buf, int len);

	    uint64_t set_center_freq(uint64_t freq);
	    uint64_t center_freq(void);
	    
	    uint32_t set_sample_rate(uint32_t sample_rate);
	    uint32_t sample_rate(void);

	    uint32_t set_bandwidth(uint32_t bandwidth);
	    uint32_t bandwidth(void);
	    
	    STATUS set_front_lna(STATUS enable);
	    STATUS front_lna(void);
	    
	    STATUS set_second_lna(STATUS enable);
	    STATUS second_lna(void);
	    
	    uint8_t set_rx_gain(uint8_t gain);
	    uint8_t rx_gain(void);
	    
	    uint8_t set_step_attenuator(uint8_t step_atten);
	    uint8_t step_attenuator(void);
	    
	    GAIN_MODE set_rx_gain_mode(GAIN_MODE mode);
	    GAIN_MODE rx_gain_mode(void);

	private:
	    // maveriq parameters
	    uint64_t d_rx_freq;
	    uint32_t d_rx_sample_rate;
            uint32_t d_rx_bandwidth;
            uint32_t d_rx_actual_bandwidth;
	    STATUS d_rx_front_lna_status;
	    STATUS d_rx_second_lna_status;
	    uint8_t d_rx_gain;
	    uint8_t d_rx_step_atten;
	    GAIN_MODE d_rx_gain_mode;

	    STATUS d_srfs_src_status;

	    // sockets
	    int d_sock;
	    int d_iq_sock;
	    int d_iq_port;
	    int d_src_port;
	    state d_state;
	    hostent *d_server;
	    sockaddr_in d_server_addr;
	    sockaddr_in d_iq_server_addr;
	    socklen_t d_addr_len;

	    bool first; // indicates if first block of data

	    // string to srfs_param_t map
	    typedef std::map<const std::string, srfs::srfs_param_t> param_map;
	    // pair string to srfs_param_t
	    typedef std::pair<const std::string, srfs::srfs_param_t> param_pair;

	    // map of valid maveriq parameters and associated strings
	    param_map maveriq_params;

	    // establishes connection to SRFS
	    void open_srfs();
	    // closes connection to SRFS
	    void close_srfs();
	    // initializes maveriq_params map
	    void init_srfs_params(void);
	    // adds a parameter to the maveriq_params map
	    void add_srfs_param( const std::string token,
				 srfs::SRFS_DATATYPES data_type,
				 void *p_value,
				 int64_t min_value,
				 int64_t max_value,
				 float resolution,
				 const std::string *p_strings );

	    // configures parameter specified by token to the value provided
	    void set_param( const std::string token, void *pValue );

	    // configures all maveriq parameters
	    void config_src();	
	    // sends a message to maveriq
	    void send_msg( char * );
	    // receive a response from maveriq
	    int receive_msg( char *, int );
	};
    } // namespace maveriq
} // namespace gr

#endif
