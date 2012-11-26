#include <avr/pgmspace.h>
#include <string.h>
#include "subD.h"
#include "../uip/uip.h"
#include "../../Hardware/Gpio.h"
#include "../TcpApps/HttpD.h"
static struct uip_udp_conn *sub_conn = NULL;

static uint16_t CopyString(uint8_t** ppBuffer, uint8_t  *pString){
	uint16_t nBytes = 0;
	char Character;

	while ((Character = *pString) != '\0')	{
		**ppBuffer = Character;
		*ppBuffer = *ppBuffer + 1;
		pString = pString + 1;
		nBytes++;
	}
	return nBytes;
}
void sendSubMessage() {
	subMsg *m = (subMsg *)uip_appdata;

	m->type = HTONS(PKT_DATA);
	m->namelength = HTONS(4);
	
//FIXME
	m->alldata[0]  = '/';
	m->alldata[1]  = 'i';
	m->alldata[2]  = 'o';
	m->alldata[3]  = 0;
	m->alldata[4]  = '0' + GpioGetPin(0);
	m->alldata[5]  = '0' + GpioGetPin(1);
	m->alldata[6]  = '0' + GpioGetPin(2);
	m->alldata[7]  = '0' + GpioGetPin(3);
	m->alldata[8]  = '0' + GpioGetPin(4);
	m->alldata[9]  = '0' + GpioGetPin(5);
	m->alldata[10] = '0' + GpioGetPin(6);
	m->alldata[11] = '0' + GpioGetPin(7);
	m->alldata[12] = '0' + GpioGetPin(8);
	m->alldata[13] = '0' + GpioGetPin(9);
	m->alldata[14] = '0' + GpioGetPin(10);
	m->alldata[15] = '0' + GpioGetPin(11);
	m->alldata[16] = '0' + GpioGetPin(12);
	m->alldata[17] = '0' + GpioGetPin(13);
	m->alldata[18] = '\n';

	
	uip_udp_send(MAX_PACKETLEN);
}

void subDInit()
{
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
}

void subDCall(subMsg *msg, uint16_t len)
{
	if (uip_poll()) {
		//if (time_req == 0 || time <= 100) {
			sendSubMessage();
		//}
	}

	if(uip_newdata()) {
		// received new data
		//time = msg->tx_timestamp_i[0];
	}
}

