/**
 * \file configure.cpp
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 * \brief client for configuring other clients via subserver. This client 
 * reads in a config file and when other clients request a value, it sends
 * the right value via subserver.
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

#include "config.h"

void cleanexit() {
	eprintf("clean exit\n");
}

void sigint_handler(int sig) {
	eprintf("received %s(%d)\n", SIGNAME[sig], sig);
}

#define CONFIGURE_ABO "/configure"

int main(int argc, char *argv[]) {
	atexit(cleanexit);
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);
	packet_t packet;
	sclient me;

	if (!me.isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}
	#define BUFSIZE	256
	char buffer[BUFSIZE];
	int  buflen=0;

	// subscribe packet
	packet.setName(CONFIGURE_ABO);
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

	config* myconf = new config("system.conf");

	while ( true ) {
		timeout.tv_sec=5;
		timeout.tv_usec=0;

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
				//printf("%1$.*2$s", rxpacket.data(), rxpacket.datalen());
				sprintf(buffer,"%1$.*2$s", rxpacket.data(), rxpacket.datalen());
				if( strncmp(buffer, "#reconfigure",12)==0 ){
					delete myconf;
					myconf = NULL;
					myconf = new config("system.conf");
					printf(" reconfigure\n");
					continue;
				}else if( (strncmp(buffer, "#get double",11)==0) && (strlen(buffer) > 12) ){
					std::string buffer_string = buffer;
					buffer_string = buffer_string.substr(buffer_string.find("get double")+11 
									,std::string::npos);
					buffer_string.replace(buffer_string.find('\n'), 1, "");
					std::string section = buffer_string.substr(0, buffer_string.find(" "));
					std::string value  = buffer_string.substr(buffer_string.find(" ")+1,
									 std::string::npos);
					printf(" get double [%s][%s]\n", section.c_str(), value.c_str());
					me.aprintf(CONFIGURE_ABO, "%lf\n", 
						myconf-> getDouble(section.c_str(), value.c_str()));	
					continue;
				}else if( (strncmp(buffer, "#get int",8)==0) && (strlen(buffer) > 8) ){
					std::string buffer_string = buffer;
					buffer_string = buffer_string.substr(buffer_string.find("get int")+8 
									,std::string::npos);
					buffer_string.replace(buffer_string.find('\n'), 1, "");
					std::string section = buffer_string.substr(0, buffer_string.find(" "));
					std::string value  = buffer_string.substr(buffer_string.find(" ")+1,
									 std::string::npos);
					printf(" get int [%s][%s]\n", section.c_str(), value.c_str());
					me.aprintf(CONFIGURE_ABO, "%d\n", 
						myconf-> getInt(section.c_str(), value.c_str()));	
					continue;
				}else if( (strncmp(buffer, "#get string",11)==0) && (strlen(buffer) > 12) ){
					std::string buffer_string = buffer;
					buffer_string = buffer_string.substr(buffer_string.find("get string")+11
									,std::string::npos);
					buffer_string.replace(buffer_string.find('\n'), 1, "");
					std::string section = buffer_string.substr(0, buffer_string.find(" "));
					std::string value  = buffer_string.substr(buffer_string.find(" ")+1,
									 std::string::npos);
					printf(" get string [%s][%s]\n", section.c_str(), value.c_str());
					me.aprintf(CONFIGURE_ABO, "%s\n", 
						myconf-> getChar(section.c_str(), value.c_str()));	
					continue;
				}else if( (strncmp(buffer, "#set double",11)==0) && (strlen(buffer) > 12) ){
					std::string buffer_string = buffer;
					buffer_string = buffer_string.substr(buffer_string.find("set double")+11
									,std::string::npos);
					buffer_string.replace(buffer_string.find('\n'), 1, "");
					std::string section = buffer_string.substr(0, buffer_string.find(" "));
					int pos = buffer_string.find(" ")+1;
					std::string value  = buffer_string.substr(pos, buffer_string.find(" ", pos)-pos);
					pos = buffer_string.find(" ", pos)+1;
					std::string insert = buffer_string.substr(pos, std::string::npos);
					printf(" set double [%s][%s] = '%s'\n", section.c_str(), value.c_str(),
								 insert.c_str());
					myconf-> setDouble(section.c_str(), value.c_str(), atof(insert.c_str()));
					continue;
				}
			}				
		} // end if noready > 0
	} // end main loop
	//eprintf("client exiting.\n");
	return 0;
}



