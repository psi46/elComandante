/*
 * sublpt.cpp
 *
 * Dennis Terhorst
 * Tue Jul  8 22:45:53 CEST 2008
 */

#include <stdlib.h>
#include <unistd.h>	// gethostname

#include <sclient.h>
#include <signalnames.h>
#include <daemon.h>	// signal
#include <string.h>	// strncat
#include "lpt.h"
#include <cmdint/cmdint.h>
#include <abo.h>

#include <sys/select.h>

volatile int wantexit=0;

void sigHandler(int sig) {
	eprintf("received %s(%d)\n", SIGNAME[sig], sig);
	wantexit++;
}

int main(int argc, char* argv[]) {

	//signal(SIGINT,  sigHandler);
	// Behavior compatible with BSD signal semantics will make certain
	// system calls restart across signals. Signal handlers installed
	// with signal() will make recvmsg() restart without return. (This
	// causes wantexit variable to be ineffective).
	//
	// catch_signals will install the handler with sigaction()
	catch_signals(sigHandler);

	if (argc<2) {
		eprintf("Usage:\n\t%s <parport-device>\n\n", argv[0]);
		exit(1);
	}

	// open parport given on command line
	lpt L(argv[1]);
	if ( L.getfd() < 0 ) {
		eperror("parport error");
		exit(1);
	}
	eprintf("sublpt opened parport at fd%d\n", L.getfd());

	// set up command interpreter
	cmdint cint;
	cint.add1("setdata", &L, &lpt::setdata);

	// decide our abo and submission name...
	char aboname[MAX_NAMELEN]={0};
	aboname[0]='/';					// start with slash
	gethostname(&(aboname[1]), MAX_NAMELEN-1);	// append hostname
	aboname[MAX_NAMELEN-1]=0;	// gethostname may be not terminated

	char* parport = rindex(argv[1], '/');		// from last slash
	if ( parport== NULL ) {			// or append "/"+complete argv[1]
		strncat(aboname, "/", MAX_NAMELEN-strlen(aboname));
		parport=argv[1];
	}
	strncat(aboname, parport, MAX_NAMELEN-strlen(aboname));
	eprintf("my name will be `%s`\n", aboname);

	// initialize subclient
	sclient me;
	if (!me.isOK()) {
		eprintf("could not initialize sclient, aborting!\n");
		return -1;
	}

	// ... and subscribe
	me.setid("sublpt");
	me.setDefaultSendname(aboname);
	me.subscribe(aboname);

	packet_t packet;
	int ret;

	fd_set readfds,allfds;
	FD_ZERO(&allfds);
	FD_SET(me.getfd(), &allfds);
	int maxfd = me.getfd();
	struct timeval timeout;
	int readyfds;

	while ( wantexit==0 ) {
		readfds = allfds;
		printf("waiting for packet...\n");
		switch ( (readyfds = select(maxfd+1, &readfds, NULL, NULL, &timeout)) ) {
		case -1:	// error
			perror("select error");
			wantexit++;
			continue;
		case 0:		// timeout
			printf("timeout\n");
			me.printf("0x%02X\n", L.getsetdata());
			timeout.tv_sec = 3;
			timeout.tv_usec= 0;
			break;
		default:
			if ( FD_ISSET(me.getfd(), &readfds) ) {
				printf("reading packet...\n");
				if ( me.recvpacket(packet) < 0 ) {
					eperror("read packet failed");
					if ( errno == ECONNREFUSED ) break;
					if ( errno == ECONNABORTED ) break;
					if ( errno == ECONNRESET   ) break;
					continue;
				};
				if ( packet.type != PKT_SETDATA ) {
					eprintf("dropping non-PKT_SETDATA packet!\n");
					continue;
				}
				printf("parsing packet...\n");
				ret = cint.execute( packet.data() );
				//if (ret < 0 )
				eperror("exec \"%s\" returned %d", packet.data(), ret);
			} else {	// unknown fd!?!
				printf("error: unknown fd ready\n");
				wantexit++;
				continue;
			} // end if
		} // end switch
	}; // end main loop
	printf(".\n");

	return 0;
}// end main
