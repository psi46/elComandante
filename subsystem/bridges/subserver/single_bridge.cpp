/**
 * single_bridge.cpp
 * Single abo bridge
 * original client by Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * modifications for this client Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>	// read, STD*_FILENO

#include <stdlib.h>	// atexit
#include <signal.h>	// signal()
#include <sys/select.h>	// select()

#include <string>	// memcpy

#include "../../sclient.h"
#include "../../packet_t.h"
#include "../../error.h"
#include "../../signalnames.h"

// set local address to one of INADDR_ANY, INADDR_LOOPBACK, inet_aton(), inet_addr(), gethostbyname()

#define SVR_ADDR		"134.61.14.104"
#define SVR_PORT		12334

// #define DUAL_WAY_BRIDGE // from makefile

//volatile int wantexit = 0;

void cleanexit() {
	eprintf("clean exit\n");

//	wantexit = 1;
}

void sigint_handler(int sig) {
	eprintf("received %s(%d)\n", SIGNAME[sig], sig);
//	wantexit = 1;
}

#define SUB_PORT 12334

int main(int argc, char *argv[]) {
	atexit(cleanexit);
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);
	packet_t packet;
	
	#define BUFSIZE	256
	char buffer[BUFSIZE];
	int  buflen=0;

	// get name of packets
	if ( argc != 5 ) {
		#ifndef DUAL_WAY_BRIDGE
			printf("Usage: %s [from server][from abo][to server][to abo]\n", argv[0]);
		#else
			printf("Usage: %s [server1][abo1][server2][abo2]\n", argv[0]);
		#endif
		exit(1);
	} 
	sclient from(argv[1],SUB_PORT);
	if (!from.isOK()) {
		eprintf("could not initialize from sclient!\n");
		return -1;
	}
	sclient to(argv[3],SUB_PORT);
	if (!to.isOK()) {
		eprintf("could not initialize to sclient!\n");
		return -1;
	}

	// subscribe packet
	packet.setName(argv[2]);
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	from.sendpacket(packet);
	from.setid(argv[0]);	// set default name

	packet.setName(argv[4]);
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	to.sendpacket(packet);
	to.setid(argv[0]);	// set default name

	packet.type = PKT_DEFAULTTYPE;

	fd_set* readfds = new fd_set();
	fd_set* writefds= new fd_set();
	fd_set* errorfds= new fd_set();
	fd_set* allfds  = new fd_set();

	int maxfd;

	FD_ZERO(allfds);
	FD_SET(from.getfd(), allfds);
	#ifdef DUAL_WAY_BRIDGE
		#warning Compiling DUAL_WAY_BRIDGE
		FD_SET(to.getfd(), allfds);
	#endif

	FD_SET(STDIN_FILENO, allfds);
	maxfd=STDIN_FILENO;

	if (from.getfd() > maxfd) maxfd=from.getfd();
	
	struct timeval timeout;
	int noready;
	int packet_cnt = 1; // avoid loop
//	while ( !wantexit ) {
	while ( true ) {
		timeout.tv_sec=5;
		timeout.tv_usec=0;

	//	memcpy(readfds, allfds, sizeof(fd_set));
		*readfds = *allfds;
		// SELECT CALL
		noready = select(maxfd+2, readfds, writefds, errorfds, &timeout);

		if ( noready < 0 ) {	///// ERROR
			eprintf("select returned %d, error %d: %s \n", noready, errno, strerror(errno));
			break;
		} else if ( noready== 0 ) {	///// TIMEOUT
			// boring....
		} else {// if noready > 0)
			if ( FD_ISSET(STDIN_FILENO, readfds) ) {
				buffer[0]=0;
				fgets(buffer, BUFSIZE, stdin);
				buflen = strlen(buffer);
				if ( buffer[0] == 'x' || feof(stdin)) break;
			}else if ( FD_ISSET(from.getfd(), readfds) ) {//subserver -> subserver
				packet_t rxpacket;
				// read packet
				if ( from.recvpacket(rxpacket) < 0 ) {
					eprintf("Error receiving a packet\n");
					continue;
				}
				to.aprintf(argv[4],"%1$.*2$s", rxpacket.data(), rxpacket.datalen());
			}
			#ifdef DUAL_WAY_BRIDGE
			else if ( FD_ISSET(to.getfd(), readfds) ) {//subserver -> subserver
				packet_t rxpacket;
				// read packet
				if ( to.recvpacket(rxpacket) < 0 ) {
					eprintf("Error receiving a packet\n");
					continue;
				}
				from.aprintf(argv[2],"%1$.*2$s", rxpacket.data(), rxpacket.datalen());
			}
			#endif
		} // end if noready > 0
	} // end main loop
	eprintf("client exiting.\n");
	return 0;
}



