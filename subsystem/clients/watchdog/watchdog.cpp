/**
 * \file watchdog.cpp
 * \brief watchdog for subserver
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 * \author Dennis Terhorst
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>	// read, STD*_FILENO

#include <stdlib.h>	// atexit
#include <signal.h>	// signal()
#include <sys/select.h>	// select()

#include <string>	// memcpy

#include <sclient.h>
#include <packet_t.h>
#include <error.h>
#include <signalnames.h>

#include <iostream> 
#include <cstdlib> 
#include <string> 
#include <unistd.h> 

#include "ratewatch.h"
 
using namespace std; 

// #define STDIN_ENABLED


// GLOBAL VARIABLES
sclient* me = NULL;
ratewatch_t* deltaT = NULL;
volatile int wantexit = 0;

void cleanexit() {
	eprintf("clean exit\n");
	if ( me != NULL ) delete me;
	if ( deltaT != NULL ) delete deltaT;
//	wantexit = 1;
}

void sigint_handler(int sig) {
	eprintf("received %s(%d)\n", SIGNAME[sig], sig);
	wantexit++;
}


void sigalrm_handler(int sig) {
	if ( me != NULL && me->isOK() ) {
		if ( deltaT->isValid() && deltaT->isLate(5) ) {
			me->printf("ALARM: bla\n");
		}
	}
	alarm(1);
}


int main(int argc, char *argv[]) {
	#define BUFSIZE	256
	int buflen = 0;
	char buffer[BUFSIZE] = {0};
	char sub_buffer[BUFSIZE] = {0};
	char alarm_buffer[BUFSIZE] = {0};

	// Setup INTERVAL CHECKS
	struct sigaction timer;
	timer.sa_handler = sigalrm_handler;
	sigemptyset(&(timer.sa_mask));
	timer.sa_flags = 0;
	sigaction(SIGALRM, &timer, NULL);
	timer.sa_handler(SIGALRM);

	float lower_limit = 0;
	float upper_limit = 0;
	char opt=0;
	while((opt = getopt(argc, argv, "s:a:l:u:")) != -1) { 
		switch(opt) { 
			case 's':	// subscribe 
				strcpy(sub_buffer, optarg);
				printf("Subscribe %s\n", sub_buffer);
 	                	break;
			case 'a':  	// alarm abo
				strcpy(alarm_buffer, optarg);
				printf("Alarm to %s\n", alarm_buffer);
				break; 
			case 'l':	// lower limit
				lower_limit = atof(optarg);
				printf("lower limit %f\n", lower_limit);
				break;
			case 'u':	// upper limit
				upper_limit = atof(optarg);
				printf("upper limit %f\n", upper_limit);	
				break;
		}
	}
	// Check Parameters
	if ( sub_buffer[0] == 0 ) {
		eprintf("error: no subscription given!\n"); fflush(stderr);
		exit(EXIT_FAILURE);
	}
	if ( alarm_buffer[0] == 0 ) {
		eprintf("error: no alarm abo given!\n"); fflush(stderr);
		exit(EXIT_FAILURE);
	}

	// Setup SUBSERVER CONNECTION
	atexit(cleanexit);
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);
	me = new sclient();
	if (!me->isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}
	printf("Sending to %s\n", alarm_buffer);
	me->setDefaultSendname(alarm_buffer);
	printf("Subscribing %s\n", sub_buffer);
	me->subscribe(sub_buffer);
	printf("Clientid %s\n", argv[0]);
	me->setid(argv[0]);	// set default name

	// Setup RATEWATCH
	deltaT = new ratewatch_t(5);

	// Setup SELECT
	fd_set* readfds = new fd_set();
	fd_set* writefds= new fd_set();
	fd_set* errorfds= new fd_set();
	fd_set* allfds  = new fd_set();
	int maxfd=-1;
#       define FD_ADD(fd, set)   { FD_SET(fd, set); if (fd>maxfd) maxfd=fd; }
	FD_ZERO(allfds);
	FD_ADD(me->getfd(), allfds);
	FD_ADD(STDIN_FILENO, allfds);

	struct timeval timeout;
	int noready;
	while ( !wantexit ) {
		timeout.tv_sec=5;
		timeout.tv_usec=0;

//		memcpy(readfds, allfds, sizeof(fd_set));
		*readfds = *allfds;
		// SELECT CALL
		noready = select(maxfd+1, readfds, writefds, errorfds, &timeout);

		if ( noready < 0 ) {	///// ERROR
			if ( errno == EINTR ) continue; // interrupted system call
			eperror("select returned %d, error\n", noready );
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
			if ( FD_ISSET(me->getfd(), readfds) ) {
				packet_t rxpacket;
				// read packet
				if ( me->recvpacket(rxpacket) < 0 ) {
					eprintf("Error receiving a packet\n");
					continue;
				}
				printf("received: %s", rxpacket.data()); fflush(stdout);
				if ( deltaT->isEarly(3) ) {
					me->aprintf(alarm_buffer,"notice: packet on %s was early\n", sub_buffer);
				}
				if ( deltaT->isLate(3) ) {
					me->aprintf(alarm_buffer,"notice: packet on %s was late\n", sub_buffer);
				}
				deltaT->beat();
				/* 
				float value = atof(rxpacket.data());
				if(value < lower_limit){
					me->aprintf(alarm_buffer,"ALARM %f < %f\n", value, lower_limit);
				}else if(value > upper_limit){
					me->aprintf(alarm_buffer,"ALARM %f > %f\n", value, upper_limit);
				}
				*/
			}
		} // end if noready > 0
	} // end main loop

	eprintf("client exiting.\n");

	return 0;
}



