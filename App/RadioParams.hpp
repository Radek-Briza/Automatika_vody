/*
 * RadioParams.hpp
 *
 *  Created on: 21. 4. 2026
 *      Author: radek
 */

#ifndef RADIOPARAMS_HPP_
#define RADIOPARAMS_HPP_

#include "main.h"

const uint32_t CHANNEL  = 865000000; // 865 MHz
const uint8_t TX_POWER	= 5;

const uint8_t BANDWIDTH			= 0; // 125 kHz
const uint8_t SPREED_FACTOR		= 6;
const uint8_t CODE_RATE			= 1;
const uint32_t PREAMBLE_LEN        = 800;
const bool FIX_LEN				= false;
const bool CRC_ON				= true;
const bool FREQ_HOP_ON			= false;
const uint8_t HOP_PERIODE			= 0;
const bool SYMBOL_INVERTED		= true;
const uint8_t PAYLOAD_LEN			= 0;
const uint16_t SYMB_TIMEOUT		= 10000;  // pocet symbolu





#endif /* RADIOPARAMS_HPP_ */
