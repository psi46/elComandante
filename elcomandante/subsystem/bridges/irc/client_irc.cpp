/**
 * \file client_irc.cpp
 * \brief bi-directional subscriber client and irc bridge
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>	// read, STD*_FILENO

#include <stdlib.h>	// atexit
#include <signal.h>	// signal()
#include <sys/select.h>	// select()

#include <string>	// memcpy

#include <subsystem/sclient.h>
#include <subsystem/packet_t.h>
#include <subsystem/error.h>
#include <subsystem/signalnames.h>

#include "irc_client.h"

// #define STDIN_ENABLED

//volatile int wantexit = 0;

void cleanexit() {
	eprintf("clean exit\n");

//	wantexit = 1;
}

void sigint_handler(int sig) {
	eprintf("received %s(%d)\n", SIGNAME[sig], sig);
//	wantexit = 1;
}



int main(int argc, char *argv[]) {
	atexit(cleanexit);
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);
	packet_t packet;
	sclient me;

	irc_client* myirc = new irc_client("127.0.0.1");
	printf("IRC created\n");
	if (!me.isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}
	#define BUFSIZE	256
	char buffer[BUFSIZE];
	int  buflen=0;

	// get name of packets
	if ( argc == 2 ) {
		strncpy(buffer, argv[1], BUFSIZE);
		buflen=strlen(buffer);
		for (int i=0; i<buflen; i++) {
			if ( buffer[i] == 13 || buffer[i] == 10 ) {
				buffer[i]=0;
				break;
			}
		}
		buflen=strlen(buffer);
	} else {
		printf("resource name: ");
		fgets(buffer, BUFSIZE, stdin);
		buflen=strlen(buffer);
		for (int i=0; i<buflen; i++) {
			if ( buffer[i] == 13 || buffer[i] == 10 ) {
				buffer[i]=0;
				break;
			}
		}
		buflen=strlen(buffer);
		printf("connecting to resource '%s'\n", buffer);
	}

	// subscribe packet
	packet.setName(buffer);
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	me.sendpacket(packet);
	me.setid(argv[0]);	// set default name

	packet.type = PKT_DEFAULTTYPE;

	fd_set* readfds = new fd_set();
	fd_set* writefds= new fd_set();
	fd_set* errorfds= new fd_set();
	fd_set* allfds  = new fd_set();

	int maxfd;

	FD_ZERO(allfds);
	FD_SET(me.getfd(), allfds);
	FD_SET(myirc->getfd(), allfds);

	FD_SET(STDIN_FILENO, allfds);
	maxfd=STDIN_FILENO;

	if (me.getfd() > maxfd) maxfd=me.getfd();
	
	struct timeval timeout;
	int noready;
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
		} else {// if noready > 0)	///// DATA
			if ( FD_ISSET(STDIN_FILENO, readfds) ) {
				buffer[0]=0;
				fgets(buffer, BUFSIZE, stdin);
				buflen = strlen(buffer);
				if ( buffer[0] == 'x' || feof(stdin)) break;
		#ifdef STDIN_ENABLED
				if ( buffer[0] == 'd' && buffer[1] == '\n' ) {	// mgm packet hack
					eprintf("Packet type set to PKT_DATA!\n");
					packet.type = PKT_DATA;
					continue;
				}
				if ( buffer[0] == 'm' && buffer[1] == '\n' ) {	// mgm packet hack
					packet.type = PKT_MANAGEMENT;
					eprintf("Packet type set to PKT_MANAGEMENT!\nData Output suppressed\n");
					continue;
				}
				if ( buffer[0] == 's' && buffer[1] == '\n' ) {	// set packet hack
					packet.type = PKT_SETDATA;
					eprintf("Packet type set to PKT_SETDATA!\nData Output suppressed\n");
					continue;
				}
				packet.setData(buffer, buflen);

				// bridge stdin -> irc & subserver			
				if ( packet.type == PKT_DATA  && packet.datalen() > 1){
					// datalen > 1
					char buffer[256];
					sprintf(buffer, "%1$.*2$s", packet.data(), packet.datalen());
					myirc->send_msg(buffer);
				}
				me.sendpacket(packet);
		#endif
			}
			if ( FD_ISSET(me.getfd(), readfds) ) {
				packet_t rxpacket;
				// read packet
				if ( me.recvpacket(rxpacket) < 0 ) {
					eprintf("Error receiving a packet\n");
					continue;
				}
				// bridge subserver -> irc
				if ( (rxpacket.type != PKT_DATA || packet.type == PKT_DATA ) && rxpacket.datalen() > 1){
					// datalen > 1
					char buffer[256];
					sprintf(buffer, "%1$.*2$s", rxpacket.data(), rxpacket.datalen());
					myirc->send_msg(buffer);
					printf("%1$.*2$s", rxpacket.data(), rxpacket.datalen());
				}
			}
			// bridge irc -> subserver & stdout
			if ( FD_ISSET(myirc->getfd(), readfds) ) {
				int type = IRC_OTHER;
				std::string irc_msg;
				int length = myirc->recv_packet(type, irc_msg);
				if(length < 0) {
					eprintf("Error receiving a packet\n");
					continue;
				}else if(length > 0){
					//printf("IRC_RECV (%d)\n", type);
					if(type == IRC_PRIVMSG){		
						printf("%s\n",irc_msg.c_str());		
						sprintf(buffer,"%s\n", irc_msg.c_str());
						buflen = strlen(buffer);
						packet.type = PKT_DATA;
						packet.setData(buffer, buflen);
						me.sendpacket(packet);
					}
				}
			}
		} // end if noready > 0
	} // end main loop

	myirc->quit();

	eprintf("client exiting.\n");

	return 0;
}



