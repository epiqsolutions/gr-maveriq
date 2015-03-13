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


#ifndef INCLUDED_MAVERIQ_MAVERIQ_H
#define INCLUDED_MAVERIQ_MAVERIQ_H

#include <maveriq/api.h>
#include <gnuradio/sync_block.h>
#include <maveriq/maveriq_defs.h>

namespace gr {
  namespace maveriq {

    /*!
     * \brief <+description of block+>
     * \ingroup maveriq
     *
     */
      class MAVERIQ_API maveriq_source_s : virtual public gr::sync_block
    {
    public:
       typedef boost::shared_ptr<maveriq_source_s> sptr;

       /*!
        * \brief Return a shared_ptr to a new instance of maveriq::maveriq.
        *
        * To avoid accidental use of raw pointers, maveriq::maveriq's
        * constructor is in a private implementation
        * class. maveriq::maveriq::make is the public interface for
        * creating new instances.
        */
       static sptr make(const std::string ip_address, uint32_t port);

       /*! 
	* \brief Set center frequency with Hz resolution.
	* \param freq The frequency in Hz
	* \return the actual center frequency
	*
	* Set the center frequency of the Matchstiq.
	*/
       virtual uint64_t set_center_freq(uint64_t freq) = 0;

       /*! 
	* \brief Set frequency with Hz resolution.
	* \param freq The frequency in Hz
	* \return the actual center frequency
	*
	* Convenience function that uses float parameter to all 
	* engineering notation to be used in GRC.
	*/
       virtual uint64_t set_center_freq(float freq) = 0;

       /*! 
	* \brief Get center frequency with Hz resolution.
	* \return the actual center frequency
	*
	* Get the center frequency of the Matchstiq.
	*/
       virtual uint64_t center_freq(void) = 0;

       /*! 
	* \brief Set the sample rate
	* \param sample_rate The sample rate
	* \return the actual sample rate
	*
	* Set the sample rate of the Matchstiq. To calculate the rate 
	* of samples delivered, the decimation stage needs to be
	* factored in.
	*/
       virtual uint32_t set_sample_rate(uint32_t sample_rate) = 0;

       /*! 
	* \brief Set the sample rate
	* \param sample_rate The sample rate
	* \return the actual sample rate
	*
	* Convenience function that uses float parameter to all engineering
	* notation to be used in GRC.
	*/
       virtual uint32_t set_sample_rate(float sample_rate) = 0;

       /*! 
	* \brief Get the sample rate
	* \return the actual sample rate
	*
	* Get the sample rate of the Matchstiq. To calculate
	* the rate of samples delivered, the decimation stage needs to be
	* factored in.
	*/
       virtual uint32_t sample_rate(void) = 0;

       /*! 
	* \brief Enable/disable the front LNA
	* \param enable Enable/disable front LNA
	* \return state of front LNA
	*
	* Enable or disable the front gain LNA (~15 dB)
	*/
       virtual STATUS set_front_lna(STATUS enable) = 0;

       /*! 
	* \brief Get the front LNA state
	* \return state of front LNA
	*
	* Get the state of the front gain LNA (~15 dB)
	*/
       virtual STATUS front_lna(void) = 0;

       /*! 
	* \brief Enable/disable the second LNA
	* \param enable Enable/disable second LNA
	* \return state of second LNA
	*
	* Enable or disable the second gain LNA (~15 dB)
	*/
       virtual STATUS set_second_lna(STATUS enable) = 0;

       /*! 
	* \brief Get the front LNA state
	* \return state of front LNA
	*
	* Get the state of the second gain LNA (~15 dB)
	*/
       virtual STATUS second_lna(void) = 0;

       /*!
	* \brief Set the value of the Rx gain 
	* \param gain rx gain value (0-76 dB)
	* \return actual rx gain
	*
	* Set the value of the Rx gain
	*/
       virtual uint8_t set_rx_gain(uint8_t gain) = 0;

       /*!
	* \brief Get the value of the Rx gain 
	* \return actual rx gain
	*
	* Get the value of the Rx gain
	*/
       virtual uint8_t rx_gain(void) = 0;

       /*!
	* \brief Set the step attenuator
	* \param step_atten attenuator value (0-31 dB in steps of 1 dB)
	* \return step attenuator value
	* 
	* Set the value of the step attenuator
	*/
       virtual uint8_t set_step_attenuator(uint8_t step_atten) = 0;

       /*!
	* \brief Get the step attenuator
	* \return step attenuator value 
	*
	* Get the value of the step attenuator
	*/
       virtual uint8_t step_attenuator(void) = 0;

       /*!
	* \brief Set the rx gain mode
	* \param mode gain mode
	* \return gain mode
	*
	* Set the rx gain mode 
	*/
       virtual GAIN_MODE set_rx_gain_mode(GAIN_MODE mode) = 0;

       /*! 
	* \brief Get the rx gain mode
	* \return gain mode
	*
	* Get the rx gain mode
	*/
       virtual GAIN_MODE rx_gain_mode(void) = 0;

    };

  } // namespace maveriq
} // namespace gr

#endif /* INCLUDED_MAVERIQ_MAVERIQ_H */

