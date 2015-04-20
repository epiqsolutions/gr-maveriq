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

#ifndef INCLUDED_MAVERIQ_MAVERIQ_IMPL_H
#define INCLUDED_MAVERIQ_MAVERIQ_IMPL_H

#include <maveriq/maveriq_source_s.h>
#include <maveriq/maveriq_defs.h>
#include "maveriq.h"

namespace gr {
  namespace maveriq {

    class maveriq_source_s_impl;
    typedef boost::shared_ptr<maveriq_source_s_impl> maveriq_source_s_impl_sptr;

    class maveriq_source_s_impl : public maveriq_source_s
    {
    private:
	boost::scoped_ptr<maveriq> rcv;
	
    public:
	maveriq_source_s_impl(const std::string ip_address, uint32_t port);
	~maveriq_source_s_impl();

	bool stop();
	bool start();


	int work(int noutput_items,
		 gr_vector_const_void_star &input_items,
		 gr_vector_void_star &output_items);
	
	uint64_t set_center_freq(uint64_t freq);
	uint64_t set_center_freq(float freq);
	uint64_t center_freq(void);

	uint32_t set_sample_rate(uint32_t sample_rate);
	uint32_t set_sample_rate(float sample_rate);
	uint32_t sample_rate(void);

	uint32_t set_bandwidth(uint32_t bandwidth);
	uint32_t set_bandwidth(float bandwidth);
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

    };

  } // namespace maveriq
} // namespace gr

#endif /* INCLUDED_MAVERIQ_MAVERIQ_IMPL_H */

