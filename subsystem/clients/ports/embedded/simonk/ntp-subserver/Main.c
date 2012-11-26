/*
 * Main File including the main-routine with initializations-
 * and stack-routines.
 * 
 * Author: Simon Kueppers
 * Email: simon.kueppers@web.de
 * Homepage: http://klinkerstein.m-faq.de
 * */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Hardware/Enc28j60.h"
#include "Hardware/Spi.h"
#include "Net/uip/uip.h"
#include "Net/uip/uip_arp.h"
#include "Net/uip/uip_TcpAppHub.h"
#include "Net/uip/uip_UdpAppHub.h"

static uint8_t g_nPrescaler = 10;
static volatile struct
{
	uint8_t fPeriodic :1;
} g_nFlags =
{ 0 };

int main()
{
	uip_ipaddr_t IpAddr;
	

	SpiInit();
	Enc28j60Init();
	uip_arp_init();
	uip_init();
	uip_TcpAppHubInit();	
	uip_UdpAppHubInit();

	Enc28j60SetClockPrescaler(2);

	TCCR1B = (1<<WGM12)|(1<<CS12);
	//OCR1A = 19531; //12500000 / 64 / 19531,25 = 10Hz (100ms)
	
	OCR1A = 48827;
	TIMSK1 = (1<<OCIE1A);

	uip_ipaddr(IpAddr, 192, 168, 0, 93);
	uip_sethostaddr(IpAddr);
	uip_ipaddr(IpAddr, 192, 168, 0, 1);
	uip_setdraddr(IpAddr);
	uip_ipaddr(IpAddr, 255, 255, 255, 0);
	uip_setnetmask(IpAddr);
	
	sei();

	
	while (1)
	{
		uip_len = Enc28j60Receive(uip_buf);

		if (uip_len > 0)
		{
			if (((struct uip_eth_hdr *)&uip_buf[0])->type
					== htons(UIP_ETHTYPE_IP))
			{
				//uip_arp_ipin();
				uip_input();

				if (uip_len > 0)
				{
					uip_arp_out();
					Enc28j60Transmit(uip_buf, uip_len);
				}

			}
			else if (((struct uip_eth_hdr *)&uip_buf[0])->type
					== htons(UIP_ETHTYPE_ARP))
			{
				uip_arp_arpin();
				if (uip_len > 0)
				{
					Enc28j60Transmit(uip_buf, uip_len);
				}
			}
		}

		if (g_nFlags.fPeriodic)
		{
			cli();
			g_nFlags.fPeriodic = 0;
			sei();

			int i= UIP_CONNS;
			while (i)
			{
				i--;
				uip_periodic(i);
				if (uip_len > 0)
				{
					uip_arp_out();
					Enc28j60Transmit(uip_buf, uip_len);
				}
			}

/* ++ begin added by Tobias Floery */
			i = UIP_UDP_CONNS;
			while (i)
			{
				i--;
				uip_udp_periodic(i);
				if (uip_len > 0)
				{
					uip_arp_out();
					Enc28j60Transmit(uip_buf, uip_len);
				}
			}
/* ++ end */

			g_nPrescaler--;

			if (g_nPrescaler == 0)
			{
				//Every 10 seconds
				uip_arp_timer();
				g_nPrescaler = 10;
			}
		}
	}

	return 0;
}

ISR(TIMER1_COMPA_vect)
{
	g_nFlags.fPeriodic = 1;
	incTime();
}
