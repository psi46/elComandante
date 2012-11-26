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
#ifndef _WIN32
	#include <sys/select.h>	// select()
#else
	#include <winsock2.h>
#endif

#include <string.h>	// memcpy
#include <time.h> // strftime

#include "sclient.h"
#include "packet_t.h"
#include "error.h"
#include "daemon.h"

// set local address to one of INADDR_ANY, INADDR_LOOPBACK, inet_aton(), inet_addr(), gethostbyname()

#define SVR_ADDR		"134.61.14.104"
#define SVR_PORT		12334

#define HUMAN_TIME	1
#define NORMAL_TIME 0

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
#ifdef _WIN32
	sclient me(SVR_ADDR, SVR_PORT);
#else
	sclient me;
#endif	
	if (!me.isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}
	#define BUFSIZE	256
	char buffer[BUFSIZE];
	int  buflen=0;

	int time_display = NORMAL_TIME;

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

#ifndef _WIN32
	FD_SET(STDIN_FILENO, allfds);
	maxfd=STDIN_FILENO;
	if (me.getfd() > maxfd) maxfd=me.getfd();
#else	// Windows need an Socket in Select but STDIN_FILENO is a Pipe
	FD_SET(0, allfds);	// Set Socket 0 (stdin) to listen see http://www.allegro.cc/forums/thread/370013/370226
	maxfd=0;
	eprintf("maxfd %d\n",maxfd);
#endif
	
	struct timeval timeout;
	int noready;
//	while ( !wantexit ) {
	while ( true ) {
		timeout.tv_sec=5;
		timeout.tv_usec=0;
	//	memcpy(readfds, allfds, sizeof(fd_set));
		*readfds = *allfds;
		// SELECT CALL
	#ifdef _WIN32
		noready = select(maxfd+1, readfds, NULL, NULL, NULL);
		if (noready == SOCKET_ERROR ) {
			eprintf("SOCKET_ERROR\n");
			errno=0;
		}
	#else
		noready = select(maxfd+1, readfds, writefds, errorfds, &timeout);
	#endif
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
				if ( (buffer[0] == 'x' && buffer[1] == '\n') || feof(stdin)) break;
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
				if ( buffer[0] == 'h' && buffer[1] == '\n' ) {	// human readable time
					if( time_display == HUMAN_TIME ){
						time_display = NORMAL_TIME;
						eprintf("time display set to normal time\n");
					}else{
						time_display = HUMAN_TIME;
						eprintf("time display set to human readable format\n");
					}
					continue;
				}
				packet.setData(buffer, buflen);

				me.sendpacket(packet);
				
			}
			if ( FD_ISSET(me.getfd(), readfds) ) {
				packet_t rxpacket;
				// read packet
				if ( me.recvpacket(rxpacket) < 0 ) {
					eprintf("Error receiving a packet\n");
					continue;
				}
				//if ( rxpacket.type != PKT_DATA || packet.type == PKT_DATA ){
				if (time_display == NORMAL_TIME) {
					printf("%1$.*2$s", rxpacket.data(), rxpacket.datalen());
				} else {
					time_t timeval;
					sscanf(rxpacket.data(), "%ld %s", &timeval, buffer);
					char time_string[80];
					struct tm time_value;
				    	localtime_r(&timeval, &time_value);
					strftime(time_string, 80, "%d.%m.%y %X", &time_value);
					printf("%s\t%s\n", time_string, buffer);
				}
				//}
			}
		} // end if noready > 0
	} // end main loop

	eprintf("client exiting.\n");

	return 0;
}



