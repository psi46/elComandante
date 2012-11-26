#include <avr/pgmspace.h>
#include <string.h>
#include "ntpD.h"
#include "../uip/uip.h"
#include "../../Hardware/Gpio.h"

static struct uip_udp_conn *ntp_conn = NULL;
static unsigned char time_req = 0;

volatile uint32_t time = 0;
uint8_t timeStr[10]={0};

void sendNTPRequest() {
	ntpMsg *m = (ntpMsg *)uip_appdata;

	m->status=0xE3;
	m->type = 0x00;
	m->precision = 0xFA04;
	m->est_error = 0x00000100;
	m->est_driftrate = 0;
	m->ref_clk_ident = 0;

	m->ref_timestamp_i = 0;
	m->ref_timestamp_f = 0;
	m->orig_timestamp_i = 0;
	m->orig_timestamp_f = 0;
	m->rx_timestamp_i = 0;
	m->rx_timestamp_f = 0;
	//m->tx_timestamp_i = 0;
	m->tx_timestamp_i[0]=0;
	m->tx_timestamp_i[1]=0;
	m->tx_timestamp_i[2]=0;
	m->tx_timestamp_i[3]=0;
	m->tx_timestamp_f = 0;
	uip_udp_send(48);
}

void NtpDInit()
{
   	uip_ipaddr_t ntp_server;	
	uip_ipaddr(ntp_server, 192, 168, 0, 100);	// HOST working
   	if(ntp_conn != NULL) {
		uip_udp_remove(ntp_conn);
	}	

	ntp_conn = uip_udp_new(&ntp_server, HTONS(PORT_NTPD));

	if (ntp_conn == NULL) {
		time = 0xAA;
		return;
	} else {
		uip_udp_bind(ntp_conn, HTONS(PORT_NTPD));
	}
	time_req = 0;
	time = 0;
}

void NtpDCall(ntpMsg *msg, uint16_t len)
{
	if (uip_poll()) {
		if (time_req == 0 || time <= 100) {
			sendNTPRequest();
			time_req++;
		}
	}

	if(uip_newdata()) {
		time = msg->tx_timestamp_i[0];
		time = time << 8;
		time += msg->tx_timestamp_i[1];
		time = time << 8;
		time += msg->tx_timestamp_i[2];
		time = time << 8;
		time += msg->tx_timestamp_i[3];
	}
}

void incTime() {
	if (time%NTP_UPDATE==0) time_req = 0;
	time++;
}

unsigned char *getTime() {
	uint32_t tmp;
	unsigned char hour, min, sec;

	tmp = time;

	sec = tmp%60;
	tmp = tmp/60;
	min = tmp%60;
	tmp = tmp/60;
	hour = tmp%24 + (UTC_OFFSET);

	timeStr[0] = '0'+hour/10;
	timeStr[1] = '0'+hour%10;
	timeStr[2] = ':';
	timeStr[3] = '0'+min/10;
	timeStr[4] = '0'+min%10;
	timeStr[5] = ':';
	timeStr[6] = '0'+sec/10;
	timeStr[7] = '0'+sec%10;
	timeStr[8] = 0;

	return timeStr;
}
