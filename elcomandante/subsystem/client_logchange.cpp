/**
 * client_logchange.cpp
 * bi-directional subscriber client for saving an own log file for every run
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

#include "sclient.h"
#include "packet_t.h"
#include "error.h"
#include "signalnames.h"

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

	FILE* log_file = NULL;

	if (!me.isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}
	#define BUFSIZE	256
	char buffer[BUFSIZE];
	int  buflen=0;

	// get name of packets
	if ( argc != 3 ) {
		printf("Usage: %s <data_abo> <control_abo>\n",argv[0]);
	}

	// subscribe packet
	packet.setName(argv[1]);
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	me.sendpacket(packet);
	me.setid(argv[0]);	// set default name

	// subscribe packet
	packet.setName(argv[2]);
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

		packet_t controlpacket;

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
				controlpacket = packet;
				controlpacket.setName(argv[2]); // set name to control channel to start 
								// abo manual from console
				me.sendpacket(packet);
			#endif
			}
			//subserver
			if ( FD_ISSET(me.getfd(), readfds) ) {
				packet_t rxpacket;
				// read packet
				if ( me.recvpacket(rxpacket) < 0 ) {
					eprintf("Error receiving a packet\n");
					continue;
				}
				buffer[0]=0;
				printf("%1$.*2$s", rxpacket.data(), rxpacket.datalen());
				sprintf(buffer,"%1$.*2$s", rxpacket.data(), rxpacket.datalen());
				controlpacket = rxpacket;
			}
			//change output file
			// start run 345 -> open file run_345.dat append
			if(strcmp(controlpacket.name(),argv[2]) == 0){
				if(strncmp(buffer, "#start run",10)==0){
					if(log_file!=NULL){
						printf("run already started\n");
						continue;
					}
					std::string buffer_string = buffer;
					buffer_string = buffer_string.substr(buffer_string.find("start run")+10 
									,std::string::npos);
					buffer_string.replace(buffer_string.find('\n'), 1, "");
					// open file
					buffer_string = "run_"+buffer_string+".dat";
					printf("-> started run %s\n",buffer_string.c_str());
					log_file = fopen(buffer_string.c_str(),"aw");
					buffer[0]=0;
				}
				// stop run -> close file		
				else if(strncmp(buffer, "#stop run",9)==0){
					if(log_file==NULL){
						printf("there is no run to stop\n");
						continue;
					}
					fclose(log_file);
					printf("-> stoped run\n");
					log_file = NULL;
					buffer[0]=0;
				}
			}else{
				// output to logfile
				if(log_file!=NULL){
					fprintf(log_file,"%s", buffer);
				}
			}				
		} // end if noready > 0
	} // end main loop
	eprintf("client exiting.\n");
	if(log_file!=NULL){
		fclose(log_file);
	}
	return 0;
}



