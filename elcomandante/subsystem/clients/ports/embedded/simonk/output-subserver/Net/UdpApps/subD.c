#include <avr/pgmspace.h>
#include <string.h>
#include "subD.h"
#include "../uip/uip.h"
#include "../../Hardware/Gpio.h"
#include "../TcpApps/HttpD.h"
static struct uip_udp_conn *sub_conn = NULL;
void sendSubscribe() {
	subMsg *m = (subMsg *)uip_appdata;
	m->type = HTONS(PKT_SUBSCRIBE);
	m->namelength = HTONS(4);
//FIXME
	m->alldata[0]  = '/';
	m->alldata[1]  = 'i';
	m->alldata[2]  = 'o';
	m->alldata[3]  = 0;
	m->alldata[4]  = '\n';
	uip_udp_send(9);
}

void subDInit(){
	subd_state = 0;
   	uip_ipaddr_t sub_server;	
	uip_ipaddr(sub_server, 192, 168, 0, 100);	// HOST working
   	if(sub_conn != NULL) {
		uip_udp_remove(sub_conn);
	}	

	sub_conn = uip_udp_new(&sub_server, HTONS(PORT_SUBD));

	if (sub_conn == NULL) {
		//FIXME handle error 
		return;
	} else {
		uip_udp_bind(sub_conn, HTONS(PORT_SUBD));
	}
	subd_state = 0;
	sendSubscribe();
}

void subDCall(subMsg *msg, uint16_t len){
	if (uip_poll()) {
		if(subd_state <= 2){
			sendSubscribe();
			subd_state++;
		}
	}
	if(uip_newdata()) {
		// received new data
		GpioSetPort(0, msg->alldata[4] - '0');
		GpioSetPort(1, msg->alldata[5] - '0');
		GpioSetPort(2, msg->alldata[6] - '0');
		GpioSetPort(3, msg->alldata[7] - '0');
		GpioSetPort(4, msg->alldata[8] - '0');
		GpioSetPort(5, msg->alldata[9] - '0');
		GpioSetPort(6, msg->alldata[10] - '0');
		GpioSetPort(7, msg->alldata[11] - '0');
		GpioSetPort(8, msg->alldata[12] - '0');
		GpioSetPort(9, msg->alldata[13] - '0');
		GpioSetPort(10, msg->alldata[14] - '0');
		GpioSetPort(11, msg->alldata[15] - '0');
		GpioSetPort(12, msg->alldata[16] - '0');
		GpioSetPort(13, msg->alldata[17] - '0');
		// echo to /oi
		msg->alldata[1]  = 'o';
		msg->alldata[2]  = 'i';
		uip_udp_send(MAX_PACKETLEN);
	}
}

