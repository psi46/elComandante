/*
 * Implements SubServer Client
 * 
 * Author: Jochen Steinmann
 * Email: jochen.steinmann@web.de
 * */

#ifndef SUBD_H_
#define SUBD_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#define PORT_SUBD 12334
// update interval in seconds	//FIXME
#define SUB_UPDATE 60

// copied from packet_t.h
#define PKT_MANAGEMENT	0
#define PKT_DATA	1
#define PKT_SUBSCRIBE	2
#define PKT_UNSUBSCRIBE 3
#define PKT_SUPPLY	4
#define PKT_UNSUPPLY	5
#define PKT_CLIENTTERM	6
#define PKT_SERVERTERM	7
#define PKT_SETDATA	8

#define PKT_HEADLENGTH	4
#define MAX_PACKETLEN	PKT_HEADLENGTH + 19

typedef struct t_SubMsg {
	uint16_t type;
	uint16_t namelength;				
	uint8_t alldata[MAX_PACKETLEN-PKT_HEADLENGTH];
} subMsg;

void subDInit();
void subDCall(subMsg *msg, uint16_t len);
void sendSubMessage();

#endif /*SUB_H_*/
