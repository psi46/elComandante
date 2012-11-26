/**
 * \file config.cpp
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#include "sub_config.h"
#include <stdlib.h>

sub_config::sub_config(char* config_abo){
	myAbo = config_abo;
	packet.setName(myAbo);
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	me.sendpacket(packet);
	me.setid("sub_config");
	packet.type = PKT_DEFAULTTYPE;
}

sub_config::~sub_config(){

}
int sub_config::getInt(const char* ch_group, const char* value){
	me.aprintf(myAbo, "#get int %s %s\n", ch_group, value);	
	packet_t rxpacket;
	if ( me.recvpacket(rxpacket) < 0 ) {
		eprintf("Error receiving a packet\n");
	}
	return atoi(rxpacket.data());
}
double sub_config::getDouble(const char* ch_group, const char* value){
	me.aprintf(myAbo, "#get double %s %s\n", ch_group, value);	
	packet_t rxpacket;
	if ( me.recvpacket(rxpacket) < 0 ) {
		eprintf("Error receiving a packet\n");
	}
	return atof(rxpacket.data());
}
const char* sub_config::getChar(const char* ch_group, const char* value){
	me.aprintf(myAbo, "#get string %s %s\n", ch_group, value);	
	packet_t rxpacket;
	if ( me.recvpacket(rxpacket) < 0 ) {
		eprintf("Error receiving a packet\n");
	}
	return rxpacket.data();
}

