/* -*- c++ -*- */
/* 
 * Copyright 2013 <+YOU OR YOUR COMPANY+>.
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
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "maveriq_source_s_impl.h"

#define     MAVERIQ_SAMPLES_PER_PACKET        (1024)
// *2 for I & Q
#define BUF_LEN      (MAVERIQ_SAMPLES_PER_PACKET*sizeof(short)*2)


namespace gr {
  namespace maveriq {

      maveriq_source_s::sptr
      maveriq_source_s::make(const std::string ip_address, uint32_t port)
      {
	  return gnuradio::get_initial_sptr 
	      (new maveriq_source_s_impl(ip_address, port));
      }
      
      /*
       * The private constructor
       */
      maveriq_source_s_impl::maveriq_source_s_impl(const std::string ip_address,
						   uint32_t port)
	  : gr::sync_block("maveriq_source_s",
                           gr::io_signature::make(0, 0, 0),
                           gr::io_signature::make(1, 1, sizeof (short)))
      {
	  rcv.reset( new maveriq(ip_address.c_str(), port) );
	  set_output_multiple(MAVERIQ_SAMPLES_PER_PACKET*2);
      }
      
      /*
       * Our virtual destructor.
       */
      maveriq_source_s_impl::~maveriq_source_s_impl()
      {
	  rcv->stop();
      }

      bool 
      maveriq_source_s_impl::stop()
      {
	  rcv->stop();
	  return true;
      }

      bool 
      maveriq_source_s_impl::start()
      {
	  rcv->start();
	  return true;
      }

      uint64_t 
      maveriq_source_s_impl::set_center_freq(uint64_t freq)
      {
	  return (rcv->set_center_freq(freq));
      }

      uint64_t 
      maveriq_source_s_impl::set_center_freq(float freq)
      {
	  return (rcv->set_center_freq(freq));
      }

      uint64_t 
      maveriq_source_s_impl::center_freq(void)
      {
	  return (rcv->center_freq());
      }

      uint32_t 
      maveriq_source_s_impl::set_sample_rate(uint32_t sample_rate)
      {
	  return (rcv->set_sample_rate(sample_rate));
      }

      uint32_t 
      maveriq_source_s_impl::set_sample_rate(float sample_rate)
      {
	  return (rcv->set_sample_rate(sample_rate));
      }

      uint32_t 
      maveriq_source_s_impl::sample_rate(void)
      {
	  return (rcv->sample_rate());
      }

      uint32_t 
      maveriq_source_s_impl::set_bandwidth(uint32_t bandwidth)
      {
	  return (rcv->set_bandwidth(bandwidth));
      }

      uint32_t 
      maveriq_source_s_impl::set_bandwidth(float bandwidth)
      {
	  return (rcv->set_bandwidth(bandwidth));
      }

      uint32_t 
      maveriq_source_s_impl::bandwidth(void)
      {
	  return (rcv->bandwidth());
      }
	
      STATUS 
      maveriq_source_s_impl::set_front_lna(STATUS enable)
      {
	  return (rcv->set_front_lna(enable));
      }

      STATUS 
      maveriq_source_s_impl::front_lna(void)
      {
	  return (rcv->front_lna());
      }
	
      STATUS 
      maveriq_source_s_impl::set_second_lna(STATUS enable)
      {
	  return (rcv->set_second_lna(enable));
      }

      STATUS 
      maveriq_source_s_impl::second_lna(void)
      {
	  return (rcv->second_lna());
      }
      
      uint8_t 
      maveriq_source_s_impl::set_rx_gain(uint8_t gain)
      {
	  return (rcv->set_rx_gain(gain));
      }

      uint8_t 
      maveriq_source_s_impl::rx_gain(void)
      {
	  return (rcv->rx_gain());
      }
	
      uint8_t 
      maveriq_source_s_impl::set_step_attenuator(uint8_t step_atten)
      {
	  return (rcv->set_step_attenuator(step_atten));
      }

      uint8_t 
      maveriq_source_s_impl::step_attenuator(void)
      {
	  return (rcv->step_attenuator());
      }

      GAIN_MODE 
      maveriq_source_s_impl::set_rx_gain_mode(GAIN_MODE mode)
      {
	  return (rcv->set_rx_gain_mode(mode));
      }

      GAIN_MODE 
      maveriq_source_s_impl::rx_gain_mode(void)
      {
	  return (rcv->rx_gain_mode());
      }

      
      int
      maveriq_source_s_impl::work(int noutput_items,
				  gr_vector_const_void_star &input_items,
				  gr_vector_void_star &output_items)
      {
	  signed short* out1 =(signed short*) output_items[0];
	  char buffer[BUF_LEN];
	  int num_bytes_rcvd = 0;
	  int out_idx = 0;
	  int recv_result = 0;
	
	  for(int i=0; i<floor(noutput_items*1.0/(2*MAVERIQ_SAMPLES_PER_PACKET));i++)
	  { 
	      recv_result = rcv->read( &buffer[0], BUF_LEN );
	      if( recv_result < 0 )
	      {
		  // no data available, send back 0
		  goto end_work;
	      }
	      num_bytes_rcvd += recv_result;
	      memcpy(&out1[out_idx], buffer, MAVERIQ_SAMPLES_PER_PACKET * sizeof(short)*2);
              out_idx = num_bytes_rcvd/2;
	  }	
	  
      end_work:
	  return (num_bytes_rcvd/2);
      }
     
  } /* namespace maveriq */
} /* namespace gr */

