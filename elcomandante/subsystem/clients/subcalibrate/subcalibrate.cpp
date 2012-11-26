/**
 * client.cpp
 * bi-directional subscriber client
 * Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * 18.Feb.2008
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>	// read, STD*_FILENO

#include <stdlib.h>	// atexit
#include <signal.h>	// signal()
#include <sys/select.h>	// select()
#include <string.h>	// memcpy

#include "../../sclient.h"
//#include "packet_t.h"
#include "../../error.h"
#include "../../signalnames.h"
#include "../../command_t.h"

#include "transfer_function_t.h"

void cleanexit() {
	eprintf("clean exit\n");
}

void sigint_handler(int sig) {
	eprintf("received %s(%d)\n", SIGNAME[sig], sig);
}


//1204711272 19.369999 33.881172 18.669998 33.714184 1021.945251 29.325514
/*struct breakup output[] = { 	{"/thpc/temperature1",	1, cal_none,	cal_none},
				{"/thpc/humidity1",	2, cal_none,	cal_none},
				{"/thpc/temperature2",	3, cal_none,	cal_none},
				{"/thpc/humidity2",	4, cal_none,	cal_none},
				{"/thpc/luftdruck",	5, cal_none,	cal_none},
				{"/thpc/druckdose",	6, cal_none,	cal_none}
			  };*/
			  
transfer_function_t* output[6];
int nooutputs = 0;

void init_calibrations() {
	output[nooutputs++] = new linear_cal("/thpc/temperature1", "/thpc/raw_data", 1);
	output[nooutputs++] = new linear_cal("/thpc/humidity1", "/thpc/raw_data", 2);
	output[nooutputs++] = new linear_cal("/thpc/temperature2", "/thpc/raw_data", 3);
	output[nooutputs++] = new linear_cal("/thpc/humidity2", "/thpc/raw_data", 4);
	output[nooutputs++] = new linear_cal("/thpc/luftdruck", "/thpc/raw_data", 5);
	output[nooutputs++] = new linear_cal("/thpc/druckdose", "/thpc/raw_data", 6);
	return;
}

			  
#define DATA_BUFSIZE	MAX_PACKETLEN
void setPassdown(sclient* me, int argc, char* argv[]) {
	FUNCTIONTRACKER;
	eprintf("no passdown defined!\n");
	char buffer[DATA_BUFSIZE];
	int  buflen=0;

	return;
}

void setCalibration(sclient *me, int argc, char* argv[]) {
	FUNCTIONTRACKER;
	if (argc < 3) {
		eprintf("%s:%d: calibrate statement wrong!\n", __FILE__, __LINE__);
		return;
	}

	bool found = false;			// find corresponding output
	for (int i =0; i<nooutputs; i++) {
		if ( strcmp(output[i]->name, argv[1]) == 0 ) {
			found = true;
		}
	}
	if (found == false) {
		eprintf("set calibration parameter unknown '%s'\n", argv[1]);
		return;
	}
	return;
}

void dataSplit(sclient *me, int datac, char* datav[]) {
	char buffer[DATA_BUFSIZE];
	int  buflen=0;
	packet_t packet;
	for (int i=0; i<nooutputs; i++) {
		buflen = snprintf(buffer, DATA_BUFSIZE,"%s\t%lf\n",
				datav[0],
				output[i]->calibration(datav[output[i]->raw_arg])
				);
		packet.setName(output[i]->name);
		packet.setData(buffer, buflen);
		packet.type = PKT_DATA;
		me->sendpacket(packet);
	}
}


int main(int argc, char *argv[]) {

	atexit(cleanexit);
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);
	
	packet_t packet;
//	sclient me(SVR_ADDR, SVR_PORT);
	sclient me;
	if (!me.isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}

	// subscribe packet
	packet.setName("/thpc/raw_data");
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	me.sendpacket(packet);

	init_calibrations();

	for (int i=0; i<nooutputs; i++) {
		packet.setName(output[i]->name);
		packet.setData("",0);
		packet.type = PKT_SUBSCRIBE;
		me.sendpacket(packet);
	}

	me.setid(argv[0]);	// set default name

	packet.type = PKT_DEFAULTTYPE;

	fd_set* readfds = new fd_set(); FD_ZERO(readfds);
	fd_set* writefds= new fd_set(); FD_ZERO(writefds);
        fd_set* errorfds= new fd_set(); FD_ZERO(errorfds);
	fd_set* allfds  = new fd_set(); FD_ZERO(allfds);
	int maxfd;
	//FD_SET(STDIN_FILENO, allfds);
	FD_SET(me.getfd(), allfds);
	maxfd=me.getfd(); //STDIN_FILENO;
	//if (me.getfd() > maxfd) maxfd=me.getfd();

	struct timeval timeout;
	int noready;
#define MAX_ARGS 30
	char *datav[MAX_ARGS];
	int   datac = MAX_ARGS;
	
	while ( true ) {
		timeout.tv_sec=5;
		timeout.tv_usec=0;
		memcpy(readfds, allfds, sizeof(fd_set));
		// SELECT CALL
		noready = select(maxfd+1, readfds, writefds, errorfds, &timeout);

		if        ( noready < 0 ) {	///// ERROR
			eperror("received signal");
			break;
		} else if ( noready== 0 ) {	///// TIMEOUT
			// boring....
		} else {// if noready > 0)	///// DATA
			if ( FD_ISSET(me.getfd(), readfds) ) {
				packet_t rxpacket;
				// read packet
				if ( me.recvpacket(rxpacket) < 0 ) { eprintf("Error receiving a packet\n"); continue; }
				
				switch (rxpacket.type) {
				case PKT_DATA:
					//printf("%1$.*2$s", rxpacket.data(), rxpacket.datalen());
					command_t<VOIDFUNCPTR>::parse(rxpacket.data(),rxpacket.datalen(), datac, datav);
					dataSplit(&me, datac, datav);
					break;
				case PKT_SETDATA:
					FUNCTIONTRACKER;
					command_t<VOIDFUNCPTR>::parse(rxpacket.data(),rxpacket.datalen(), datac, datav);
					if (datac > 1) {
						if        (strcmp(datav[0], "set") == 0 ) {
							setPassdown(&me, datac, datav);	
						} else if (strcmp(datav[0], "calibrate") == 0 ) {
							setCalibration(&me, datac, datav);
						} else {
							eprintf("WARNING: unknown PKT_SETDATA command\n");
						}
					} else {
						eprintf("ERROR: Could not parse PKT_SETDATA packet\n");
					}
					break;
				default:
					eprintf("WARNING: Discarding packet of unhandled type %d!\n", rxpacket.type);
				} // end select
			} else {
				eprintf("ERROR: unknown file descriptor ready for read!\n");
			} // end if FD_ISSET(sclient)
		} // end if noready > 0
	} // end main loop

	eprintf("client exiting.\n");

	return 0;
}



