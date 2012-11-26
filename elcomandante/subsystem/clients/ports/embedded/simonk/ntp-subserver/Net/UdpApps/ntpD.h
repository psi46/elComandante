/*
 * Implements RFC958 - Network Time Protocol (NTP)
 * 
 * Author: Tobias Flöry
 * Email: tobias.floerycable.vol.at
 * Homepage: http://tobiscorner.floery.net
 * 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 Copyright 2008 Tobias Floery
 * */

#ifndef NTPD_H_
#define NTPD_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#define PORT_NTPD 123
// hour offset from utc
#define UTC_OFFSET +1
// update interval in seconds
#define NTP_UPDATE 60

typedef struct t_ntpMsg {
	uint8_t status;
	uint8_t type;
	uint16_t precision;

	uint32_t est_error;
	uint32_t est_driftrate;
	uint32_t ref_clk_ident;

	uint32_t ref_timestamp_i;
	uint32_t ref_timestamp_f;
	uint32_t orig_timestamp_i;
	uint32_t orig_timestamp_f;
	uint32_t rx_timestamp_i;
	uint32_t rx_timestamp_f;
	uint8_t tx_timestamp_i[4]; 
	uint32_t tx_timestamp_f; 

} ntpMsg;

void incTime();
void NtpDInit();
void NtpDCall(ntpMsg *msg, uint16_t len);
unsigned char *getTime();
void sendNTPRequest();

#endif /*NTP_H_*/
