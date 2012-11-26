/*
 * File including all necessary settings regarding the ENc28j60
 * Ethernet Chip. 
 * PS: You shouldn't care about ENC28J60's memory settings. leave it as it is.
 * Things you can or should change are the Pin-mapping and mac-address
 * 
 * Author: Simon Kueppers
 * Email: simon.kueppers@web.de
 * Homepage: http://klinkerstein.m-faq.de
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
 
 Copyright 2008 Simon Kueppers
 * */

#ifndef ENC28J60_H_
#define ENC28J60_H_

#include <stdint.h>
#include <avr/io.h>

//Physical connection
#define ENC28J60_DDR		DDRB
#define ENC28J60_PORT		PORTB
#define ENC28J60_INT		PB1
#define ENC28J60_CS			PB2
#define ENC28J60_MOSI		PB3
#define ENC28J60_MISO		PB4
#define ENC28J60_SCK		PB5

//OnChip Buffer
//Errata Workaround: RX Buffer should start at 0x0000
//Errata Workaround: RXEND should not be even!
#define ENC28J60_RXSTART	0x0000	//6kb
#define ENC28J60_RXEND		0x17FF
#define ENC28J60_TXSTART	0x1800	//2kb
#define ENC28J60_TXEND		0x1FFF

//Ethernet
#define ENC28J60_MAXFRAME	500	//maximum frame len (bytes)
#define ENC28J60_MAC1		0x00
#define ENC28J60_MAC2		0x03
#define ENC28J60_MAC3		0x6F
#define ENC28J60_MAC4		0x55
#define ENC28J60_MAC5		0x1C
#define ENC28J60_MAC6		0xC2

//Other
#define ENC28j60_INLINE		1	//use function inlining (faster, but increases code-size)
//Function declarations
//Initialize Chip (Initialize SPI Unit before!)
void Enc28j60Init();
//Receives an Ethernet-frame if one available. Else it returns zero
//This function will never receive more than ENC28J60_MAXFRAME bytes
uint16_t Enc28j60Receive(uint8_t* pBuffer);
//Transmits a given Ethernet-frame
void Enc28j60Transmit(	const uint8_t* pBuffer,
						uint16_t nBytes);
//Use this function to control onchip clock-prescaling
//provided by the ENC28J60 for using as the main clock for host processors
//Startup default is ENC28J60's clock divided by 4 (6.25MHz)
void Enc28j60SetClockPrescaler(uint8_t nPrescaler);

#endif /*ENC28J60_H_*/
