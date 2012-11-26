#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>	// fork, close, dup, gethostname
#include <sys/select.h>
#include <time.h>
#include "selectable_sclient.h"
#include <fcntl.h>
#include <error.h>

volatile int wantexit=0;

#include "child_info_t.h"
#include "pipe.h"

// GLOBAL VARS
child_info_t *mychild;
struct cmdflags_t cmdflags = {0};
sclient* meptr = NULL;



#include <daemon.h>
#define MAX(a,b)	(a>b?a:b)
#define PARENT	0	// for fork()

//////////////////////////////////////////////////////////////////////////////////////

#include "util.h"

void clean_exit() {
	//if (meptr != NULL) delete meptr; // sclient is automatic in main
}

void sigint_handler(int sig) {
	wantexit++;
	pid_t pid;
	signal_block(SIGCHLD);
	if ( (pid=mychild->Child()->getPID()) > 0 ) {
		if ( sig == SIGINT ) kill(pid, sig);
		if ( sig == SIGTERM ) kill(pid, sig);
	}
	signal_unblock(SIGCHLD);
}

void sigalrm_handler(int sig) {
	if (meptr == NULL) return;

	// FIXME: add some stats here
	meptr->printf("%ld %s > status=%s, pid=%d\n", time(NULL), mychild->ClientID(), mychild->Child()->ChildStatusString(), mychild->Child()->getPID());
	if (child_t::getnoc() < 1) {
		wantexit++;
		fprintf(stderr, "no children left, wantexit.\n");
	}
	alarm(STATUS_INTERVAL_SEC);
}

//////////////////////////////////////////////////////////////////////////////////////

#include "signal_handling.h"
#include <errno.h>
int main(int argc, char* argv[]) {

	atexit(clean_exit);

	parse_commandline(argc, argv, cmdflags); // fills cmdflags
	if (cmdflags.rest_argc<1) { printf("not enough arguments!\n"); help();}	// we need a programm name

	if ( cmdflags.restart & ~CLD_RESET_MASK_ANY ) {
		printf("ERROR: Invalid {-r|--restart} argument! See --help.\n");
		return -1;
	}

	mychild = new child_info_t(cmdflags.rest_argc, cmdflags.rest_argv);

	// construct abo and id names
	if ( cmdflags.clientid[0]!= 0 ) mychild->setClientID(cmdflags.clientid );
	if ( cmdflags.inabo[0]   != 0 ) mychild->setInabo(   cmdflags.inabo    );
	if ( cmdflags.outabo[0]  != 0 ) mychild->setOutabo(  cmdflags.outabo   );
	if ( cmdflags.errabo[0]  != 0 ) mychild->setErrabo(  cmdflags.errabo   );

	// print info
	if ( cmdflags.daemonize ) {
		daemonize_me(); // go into background
		sleep(1); // wait for parent to settle
	} else {
		mychild->PrintInfo();
	}

	// setup subserver connection and register some abos
	sclient_selectable me;
	me.setid(mychild->ClientID());
	me.subscribe(mychild->Inabo());
	me.setDefaultSendname(cmdflags.scriptabo);
	meptr = &me;	// point global to this sclient

	// setup signal handlers	
	signal_block(SIGALRM);// block SIGALRM to finish init before first stat request
	if ( register_handler(SIGALRM, sigalrm_handler) < 0)
		{ fprintf(stderr, "ERROR: Could not set SIGALRM action. Bailing out.\n"); return 1; }
	if ( register_handler(SIGINT, sigint_handler) < 0)
		{ fprintf(stderr, "ERROR: Could not set SIGINT action. Bailing out.\n"); return 1; }
	if ( register_handler(SIGTERM, sigint_handler) < 0)
		{ fprintf(stderr, "ERROR: Could not set SIGTERM action. Bailing out.\n"); return 1; }

	do { // child restart loop
		wantexit = 0;
		mychild->Child()->dofork(); /////////////////////////////////////// SPLIT CHILD


		// FIXME: print command line parameters, too.
		me.printf("%ld %s > status=CLD_STARTED, pid=%d, uid=%d\n", time(NULL), mychild->ClientID(), mychild->Child()->getPID(), getuid());
		//me.printf("%ld %s > ppid/pid=%d/%d, daemonppid/pid=%d/%d, uid=%d\n", time(NULL), mychild->ClientID(),
		//	predaemon.ppid, predaemon.getPID(), postdaemon.ppid, postdaemon.getPID(), getuid());


		int readyfds=0;
		int rdlen=0;

		sigalrm_handler(SIGALRM);	// start status messages (blocked until select())

		while (wantexit == 0) {	/// select loop ////////////////////////////////////////////// main loop

			if (child_t::getnoc() < 1) {
				fprintf(stderr, "no children left, exiting.\n");
				break;
			}
			signal_unblock(SIGALRM);
			signal_block(SIGCHLD);
			switch ( (readyfds=selectable::run()) ) {	// FIXME: use pselect here!
			case 0:	// timeout -- will never happen
			case -1:
				if ( errno == EINTR ) { continue; } // received a signal...
				eperror("parent: select failed return=%d", readyfds);
				wantexit++;
				break;
			default:
				signal_block(SIGALRM);
#define BUFLEN	256
				// exceptions = mention on scriptabo
				if ( mychild->Child()->stderr_pipe()->isready(CHK_EXCEPT) ) {
					--readyfds;
					me.printf("%ld %s > info='exception sent on stderr file descriptor of child'\n", time(NULL), mychild->ClientID());
				}
				if ( mychild->Child()->stdout_pipe()->isready(CHK_EXCEPT) ) {
					--readyfds;
					me.printf("%ld %s > info='exception sent on stdout file descriptor of child'\n", time(NULL), mychild->ClientID());
				}
				
				// child output = write to sclient
				if ( mychild->Child()->stderr_pipe()->isready(CHK_READ) ) { //FD_ISSET(mychild->err[P_R], &rfd) ) 
					--readyfds;
					char buffer[BUFLEN] = {0};
					if ( (rdlen=read(mychild->Child()->stderr_pipe()->getfd(), &buffer, BUFLEN)) < 0 ) {
						perror("read child(err) failed"); //return 1;
						wantexit++;
					} else if ( rdlen > 0 ) {
						me.aprintf(mychild->Errabo(), "%s", buffer);
					} else {
						me.printf("%ld %s > info='child closed stderr file descriptor.'\n", time(NULL), mychild->ClientID());
						mychild->Child()->stderr_pipe()->close_read();
					}
				}
				if ( mychild->Child()->stdout_pipe()->isready(CHK_READ) ) { //FD_ISSET(mychild->out[P_R], &rfd) ) {
					--readyfds;
					char buffer[BUFLEN] = {0};
					if ( (rdlen=read(mychild->Child()->stdout_pipe()->getfd(), &buffer, BUFLEN)) < 0 ) {
						perror("read child(out) failed"); //return 1;
						wantexit++;
					} else if ( rdlen > 0 ) {
						me.aprintf(mychild->Outabo(), "%s", buffer);
					} else {
						me.printf("%ld %s > info='child closed stdout file descriptor.'\n", time(NULL), mychild->ClientID());
						mychild->Child()->stdout_pipe()->close_read();
					}
				}

				// sclient input = write to child
				if ( me.isready(CHK_READ) ) { //FD_ISSET(me.getfd(), &rfd) ) 
					--readyfds;
					int ret;
					packet_t packet;
					if ( (rdlen=me.recvpacket(packet)) < 0 ) {
						perror("receive packet failed"); //return 1;
						wantexit++;
					} else if ( rdlen > 0 ) {
						if ( strcmp(packet.name(), mychild->Inabo())==0 ) {
							if ( (ret=write(mychild->Child()->stdin_pipe()->getfd(), packet.data(), packet.datalen())) < packet.datalen() ) {
								me.printf("%ld %s > info='could not write to childs stdin!'", time(NULL), mychild->ClientID());
								perror("child stdin write failed");
								wantexit++;
							}
						}
					} else {
						printf("parent: sclient read returned zero.\n");
					}
				}

				if ( readyfds > 0) {
					me.printf("%ld %s > info='parent has %d unknown read-ready fds. bailing out.'\n",
						   time(NULL), mychild->ClientID(), readyfds );
					wantexit++;
				}
			} // end switch
			signal_unblock(SIGCHLD);
		} // end select loop

//			me.printf("%ld %s > status=%s, pid=%d\n", time(NULL), mychild->ClientID(), ChildStatusString(mychild->getStatus()), mychild->getPID());

		if ( !mychild->Child()->terminate() ) {
			printf("warning: child did not teminate.\n");
		}

		//me.printf("end parent, restart loop. wantexit=%d\n", wantexit);
	} while (    (mychild->Child()->getStatus() == CLD_KILLED && (cmdflags.restart & CLD_RESET_MASK_KILLED))
	          || (mychild->Child()->getStatus() == CLD_EXITED && (cmdflags.restart & CLD_RESET_MASK_EXITED))
	        );
	me.printf("%ld %s > info='parent process %d terminating.'\n", time(NULL), mychild->ClientID(), getpid());
	return 0;
} // main()
