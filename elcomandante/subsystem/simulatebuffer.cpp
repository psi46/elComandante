#include <subsystem/sclient.h>

#include "error.h"	// eprintf()
#include "daemon.h"	// catch_signals()
#include "stdlib.h"	// exit()

#include <time.h>	// time()
sclient* meptr=NULL;

// GLOBAL VARIABLES
double ambientpressure=0.0;
double buffersize=10.0;

// SLOT: /buffer/pressure
void on_pressure(packet_t packet) {
	time_t t;
	double p;
	if ( sscanf(packet.data(), "%ld %lf", &t, &p ) != 2 ) {
		eprintf("on_pressure(): could not parse input!");
		return;
	}
	meptr->aprintf("/buffer/fill", "%ld\t%f Liter\n", time(NULL), (p-ambientpressure)*buffersize );
}

// SLOT: /buffer/pressure
void on_ambpressure(packet_t packet) {
	time_t t;
	float p;
	if ( sscanf(packet.data(), "%ld %f", &t, &p ) != 2 ) {
		eprintf("on_pressure(): could not parse input!");
		return;
	}
	if ( 900.0<p && p<1200.0) {
		ambientpressure = p/1000.0;
		eprintf("new ambientpressure: %f bara\n", ambientpressure);
	} else {
		eprintf("input out of range: ambient pressure == %f mbara?\n", p);
	}
	//meptr->aprintf("/buffer/fill", "%ld\t%f Liter\n", time(NULL), (p-ambientpressure)*buffersize );
}

void on_fillset(packet_t packet) {
	if ( packet.type != PKT_SETDATA ) {
		eprintf("rx packet not type PKT_SETDATA!\n");
		return;
	}

	int	argc=16;
	char*	argv[16];
	command_t<VOIDFUNCPTR>::parse(packet.data(), packet.datalen(), argc, argv);
	
	// FIXME here!!
	
	return;
}
/////////////////////////////////////////////////
void cleanexit(int sig) {
	eprintf("got signal %d\n", sig);
	eprintf("unregister me at subserver\n");
	meptr->terminate();
	exit(0);
}


int main(void) {
	// initialize connection
	sclient me;
	me.setid("simulatebuffer");
	meptr = &me;

	// do some cleanup if we get a SIGINT or SIGTERM
	catch_signals( cleanexit );

	// register callbacks for do_rxcalls
	me.register_rxcall(on_pressure, "/buffer/pressure");
	me.register_rxcall(on_ambpressure, "/ambient/pressure");
	me.register_rxcall(on_fillset, "/buffer/fill");

	// read one packat at a time and do the callback
	packet_t packet;
	while (1) {
		//printf("waiting for packet\n");
		if ( me.recvpacket(packet) < 0 ) {
			printf("got error\n");
			continue;
		}
		packet.print();
		me.do_rxcalls(packet);

	}
}
