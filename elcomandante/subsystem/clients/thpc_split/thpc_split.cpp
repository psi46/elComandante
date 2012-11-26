/*
 * thpc_split.cpp
 * Dennis Terhorst
 * 13 Mar 2008
 */
#include <signal.h>
#include <time.h>
#include <unistd.h> // sleep
#include <sys/select.h>	// select()

#include "../../signalnames.h"
#include "../../sclient.h"
#include "../../error.h"

#define SELECT_TIMEOUT	10
volatile int wantexit=0;

void sigINT_handler(int sig) {
	eprintf("received %s(%d)\n", SIGNAME[sig], sig);
	wantexit=1;
	return;
}

int sendpacket(sclient* me, char* name, time_t t, double val) {
	packet_t packet;
	char buffer[MAX_PACKETLEN];
	int len = snprintf(buffer, MAX_PACKETLEN, "%d\t%lf\n", t, val);
	packet.setName(name);
	packet.setData(buffer, len);
	return me->sendpacket(packet);
}

int main(int argc, char* argv[]) {
	// init vars and signal handlers
	packet_t packet;
	if ( signal(SIGTERM, sigINT_handler) == SIG_ERR ) {
		eprintf("ERROR: could not set up signal handler for SIGTERM!\n");
		return(-1);
	}
	if ( signal(SIGINT, sigINT_handler) == SIG_ERR ) {
		eprintf("ERROR: could not set up signal handler for SIGINT!\n");
		return(-1);
	}
	// connect sclient
	sclient me;
	if (!me.isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}
	eprintf("%s initialized\n", argv[0]);
	
	// subscribe data channel
	packet.setName("/thpc/raw_data");
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	me.sendpacket(packet);
	
	me.setid("thpc_split");
	
	// select() call setup
	fd_set* readfds = new fd_set();
	fd_set* writefds= new fd_set();
        fd_set* errorfds= new fd_set();
	fd_set* allfds  = new fd_set();
	int maxfd;
	FD_ZERO(allfds);
	//FD_SET(STDIN_FILENO, allfds);
	FD_ZERO(writefds);
	FD_ZERO(errorfds);
	FD_SET(me.getfd(), allfds);
	//maxfd=STDIN_FILENO;
	//if (me.getfd() > maxfd) maxfd=me.getfd();
	maxfd=me.getfd();
	
	//
	// MAIN LOOP
	//
	int noready;
	struct timeval timeout;
	while ( ! wantexit ) {
		memcpy(readfds, allfds, sizeof(fd_set));
		timeout.tv_sec = SELECT_TIMEOUT;
		timeout.tv_usec = 0;
		// SELECT CALL
		noready = select(maxfd+1, readfds, writefds, errorfds, &timeout);

		if        ( noready < 0 ) {	///// ERROR
			if (errno == EINTR) {
				if (wantexit) {
					eprintf("terminating due to SIGINT\n");
					break;
				}
				continue;
			} else {
				eperror("%s:%d: WARNING: terminating due to select() failure", __FILE__, __LINE__);
			}
			break;
		} else if ( noready > 0 ) {	///// DATA
			if ( FD_ISSET(me.getfd(), readfds) ) {
				packet_t rxpacket;
				// read packet
				if ( me.recvpacket(rxpacket) < 0 ) {
					if (errno == ECONNREFUSED) {
						eprintf("\nERROR: SERVER IS DOWN OR REFUSED CONNECTION. ABORTING.\n\n");
						break;
					}
					eprintf("WARNING: Received erroneous packet! Discarding.\n");
					continue;
				}

				time_t sec;
				double T1,H1,T2,H2,P1,P2;
				if (sscanf(rxpacket.data(), "%d %lf %lf %lf %lf %lf %lf",
					&sec, &T1, &H1, &T2, &H2, &P1, &P2) != 7) {
					eprintf("could not parse line.");
				}
				eprintf("time:%d T1=%lf, H1=%lf, T2=%lf, H2=%lf, P1=%lf, P2=%lf\n",
					sec, T1,H1,T2,H2,P1,P2);
				sendpacket(&me, "/thpc/Temp1", sec, T1);
				sendpacket(&me, "/thpc/Humi1", sec, H1);
				sendpacket(&me, "/thpc/Temp2", sec, T2);
				sendpacket(&me, "/thpc/Humi2", sec, H2);
				sendpacket(&me, "/thpc/Pressure1", sec, P1);
				sendpacket(&me, "/thpc/Pressure2", sec, P2);
			}
		} else { //if ( noready== 0 ) {	///// TIMEOUT
			eprintf("no signal.\n");
		}
	} // end main loop
	
	eprintf("program end.\n");
	return 0;
}
