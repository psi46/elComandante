/**
 * \file subscript.cpp
 *
 * subserver client to dynamically start and stop scripts
 *
 * \author Dennis Terhorst
 * \date 13 Mar 2008
 */
//#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h> // sleep
#include <sys/select.h>	// select()
#include <sys/types.h>	// stat()
#include <sys/stat.h>	// stat()
		     
#include <subsystem/sclient.h>
#include "script_t.h"
#include <subsystem/slist.h>
#include <subsystem/command_t.h>

#define ALLUPDATE_INTERVAL	15

#define SUBSCRIPT_CMDFUNC	int (*)(int argc, char* argv[])
sclient* meptr;
slist<script_t> scriptlist;	// list of scripts
slist<command_t<SUBSCRIPT_CMDFUNC> > cmd;	// list of commands

#define SUBSCRIPT_INFO		"/subscript/info"
#define SUBSCRIPT_CONTROL	"/subscript/control"
#define SUBSCRIPT_CONF		"subscript.conf"

#define SCLIENT_OUTPUT

#ifdef SCLIENT_OUTPUT
#  ifdef eprintf
#    undef eprintf
#  endif
#  ifdef eperror
#    undef eperror
#  endif
#  include <string.h>
#  include <errno.h>
#  define eprintf(format, args...) meptr->aprintf(SUBSCRIPT_CONTROL, format , ##args)
#  define eperror(format, args...) meptr->aprintf(SUBSCRIPT_CONTROL, format ": %s\n", ##args, strerror(errno))
#endif

void printstati() {
	static bool havehead=false;
	if (!havehead)	meptr->aprintf(SUBSCRIPT_INFO, "#TIME      %s\n", script_t::Status_head_string() );
	havehead=true;
	for (int i=0; i<scriptlist.length(); i++) {
		meptr->aprintf(SUBSCRIPT_INFO, "%ld\t%s\n", time(NULL), scriptlist[i].Status_string());
	}
}

#define MAX_LINELEN	512
#define MAX_ARGS	32
int load_scripts(char* filename) {	// give subscript.conf as parameter
	FILE* fp;
	struct stat fileinfo;
	//script_t sinfo;
	
	if ( stat(filename, &fileinfo) < 0 ) {
		eperror("could not stat \"%s\"", filename);
		return 1;
	}
	
	if ( ! S_ISREG(fileinfo.st_mode) ) {
		eprintf("%s is not a regular file!", filename);
		return 1;
	}
	
	fp = fopen(filename,"r");
	if ( fp == NULL ) {
		eperror("could not open configuration file %s!", filename);
		return 1;
	}

	eprintf("loading scriptlist %s:\n", filename);

	char buffer[MAX_LINELEN];
	int argc = MAX_ARGS;
	char *argv[MAX_ARGS];
	int l=0;
	int runmode = SCRIPT_DEFAULT_RUNMODE;
	while ( ! (feof(fp) || ferror(fp)) ) {
		buffer[0] = 0;
		fgets(buffer, MAX_LINELEN, fp); l++;
		if (buffer[0] == 0) continue;	// no read
		if (buffer[0] == '#') continue;	// discard comments;
		command_t<SUBSCRIPT_CMDFUNC>::parse(buffer, strlen(buffer), argc, argv);
		if ( argc == 0 ) continue;	// empty line
		if ( argc < 2 ) {
			eprintf("not enough args (%d) in line %d\n", argc, l);
			continue;
		}
		runmode = SCRIPT_DEFAULT_RUNMODE;
		if ( argc >=2 ) {
			if        (strcmp("script", argv[2]) == 0) {
				runmode = SCRIPT_RUNMODE_SCRIPT;
			} else if (strcmp("module", argv[2]) == 0) {
				runmode = SCRIPT_RUNMODE_MODULE;
			} else {
				eprintf("unknown runmode '%s' in line %d! skipping.\n", argv[2], l);
				continue;
			}

		}
		//if (sinfo.setName(argv[0]) < 0 ) {
		//	eprintf("%s:%d: invalid scriptname '%s'\n", filename, l, argv[0]);
		//	continue;
		//}
		//if (sinfo.setStatus(argv[1][0]) < 0) {
		//	eprintf("%s:%d: invalid script status '%s'\n", filename, l, argv[1][0]);
		//	continue;
		//}
		//FUNCTIONTRACKER;	// FIXME: do some checks for the binary here!
		scriptlist.add(script_t(argv[0], argv[1], runmode));		// add scriptstatus to protocol definition
		eprintf("loaded script '%s'\n", argv[0]);
	}
	
	if ( fclose(fp) != 0 ) {
		eperror("could not close configuration file");
		return 1;
	}
	return 0;
	
}

//
// SUBSCRIPT_CMDFUNCtions
//
int cmd_list(int argc, char* argv[]) {
	printstati();
	return 0;
}

int cmd_run(int argc, char* argv[]) {
	if (argc < 2) {
		eprintf("%s:%d: ERROR: run command called with argc < 2\n", __FILE__, __LINE__);
		return -1;
	}
	
	script_t* s;
	if ( (s = scriptlist.get(script_t(argv[1],""))) == NULL) {
		eprintf("could not run \"%s\": unknown script!\n", argv[1]);
		return -1;
	}

	argv[argc]=NULL;
	if (s->run(&(argv[1])) < 0) {	// run script with argv[0]="run" removed;
		eprintf("Failed to run() %s\n\t", s->getName());
	}
	meptr->aprintf(SUBSCRIPT_INFO, "%ld\t%s\n", time(NULL), s->Status_string());	// sendpacket();
	return 0;
}

int cmd_reset(int argc, char* argv[]) {
	if (argc < 2) {
		eprintf("%s:%d: ERROR: reset command called with argc < 2\n", __FILE__, __LINE__);
		return -1;
	}
	
	script_t* s;
	if ( (s = scriptlist.get(script_t(argv[1],""))) == NULL) {
		eprintf("could not reset \"%s\": unknown script!\n", argv[1]);
		return -1;
	}
	s->reset();
	meptr->aprintf(SUBSCRIPT_INFO, "%ld\t%s\n", time(NULL), s->Status_string());	// sendpacket();
	return 0;
}

int cmd_reload(int argc, char* argv[]) {
	if (argc =! 1) {
		eprintf("%s:%d: ERROR: reset command called with argc != 1\n", __FILE__, __LINE__);
		return -1;
	}
	
	scriptlist.clear();
	load_scripts(SUBSCRIPT_CONF);
	return 0;
}

int cmd_kill(int argc, char* argv[]) {
	if (argc < 2) {
		eprintf("%s:%d: ERROR: kill command called with argc < 2\n", __FILE__, __LINE__);
		return -1;
	}
	
	script_t* s;
	if ( (s = scriptlist.get(script_t(argv[1],""))) == NULL) {
		eprintf("could not kill \"%s\": unknown script!\n", argv[1]);
		return -1;
	}

	if (s->terminate() < 0) {	// run script with argv[0]="run" removed;
		eprintf("Failed to kill() %s\n\t", s->getName());
	}
	meptr->aprintf(SUBSCRIPT_INFO, "%ld\t%s\n", time(NULL), s->Status_string());	// sendpacket();
	return 0;
}

int unknowncommand(int argc, char* argv[]) {
	if (argc >0) {
		eprintf("received unknown command \"%s\"\n", argv[0]);
	} else {
		eprintf("received empty command\n");
	}
	return 0;
}

void init_commands() {
	cmd.add( command_t<SUBSCRIPT_CMDFUNC>("ls", cmd_list) );
	cmd.add( command_t<SUBSCRIPT_CMDFUNC>("run", cmd_run) );
	cmd.add( command_t<SUBSCRIPT_CMDFUNC>("reset", cmd_reset) );
	cmd.add( command_t<SUBSCRIPT_CMDFUNC>("reload", cmd_reload) );
	cmd.add( command_t<SUBSCRIPT_CMDFUNC>("kill", cmd_kill) );
	return;
}

void run_command_packet(packet_t* packet)
{
	if (packet->type != PKT_DATA ) {
		eprintf("%s:%d Packet type not PKT_MANAGEMENT!\n", __FILE__, __LINE__);
		return;
	}

	char *argv[16];
	int argc=0;
	// gather info
	command_t<SUBSCRIPT_CMDFUNC>::parse(packet->data(), packet->datalen(), argc, argv);	// FIXME: this should not be template dependant

	bool found = false;
	if (argc >0) {
		for (int i=0; i<cmd.length(); i++) {
			if   ( strcmp(argv[0], cmd[i].name) == 0 )
			{
				(cmd[i].func)(argc, argv);
				found=true;
				break;
			}	
		}
	}
	if ( !found ) {
		unknowncommand(argc, argv);
	}

	// return
	return;
}

//
// SIGNAL HANDLERS
//
void sigCHLD_handler(int sig) {	// check all script stati
	for (int i = 0; i<scriptlist.length(); i++) {
		if ( scriptlist[i].justTerminated() ) {
			meptr->aprintf(SUBSCRIPT_INFO, "%ld\t%s\n", time(NULL), scriptlist[i].Status_string());	// sendpacket();
		}
	}
	return;
}

volatile int wantexit=0;

void sigINT_handler(int sig) {
	wantexit=1;
	return;
}

int main(void) {
	// init vars and signal handlers
	packet_t packet;
	if ( signal(SIGINT, sigINT_handler) == SIG_ERR ) {
		fprintf(stderr, "ERROR: could not set up signal handler for SIGINT!\n");
		exit(-1);
	}
	if ( signal(SIGCHLD, sigCHLD_handler) == SIG_ERR ) {
		fprintf(stderr, "ERROR: could not set up signal handler for SIGCHLD!\n");
		exit(-1);
	}
	
	// connect sclient
	sclient me;
	if (!me.isOK()) {
		fprintf(stderr, "could not initialize sclient!\n");
		return -1;
	}
	meptr = &me;
	
	me.setid("subscript");
	me.setDefaultSendname("/subscript/control");

	eprintf("subscript client initialized\n");
	
	//
	// setup scripts
	//
	eprintf("setting up scripts...\n");
	load_scripts(SUBSCRIPT_CONF);
	/*
	scriptlist.add( script_t("truetest", "/bin/true") );
	scriptlist.add( script_t("sleeptest", "./sleepscript.sh") );
	scriptlist.add( script_t("thpc_split", "./thpc_split") );
	scriptlist.add( script_t("thpc_calibration", "./subcalibrate") );
	scriptlist.add( script_t("falsetest", "/bin/false") );
	scriptlist.add( script_t("falsetest", "/bin/false") );
	*/
	printstati();

	// init controll commands
	init_commands();
	//
	// subscribe control channel
	//
	packet.setName("/subscript/control");
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	me.sendpacket(packet);
	
	//
	// select() call setup
	//
	fd_set* readfds = new fd_set();
	fd_set* writefds= new fd_set();
        fd_set* errorfds= new fd_set();
	fd_set* allfds  = new fd_set();
	int maxfd;
	FD_ZERO(allfds);
	//FD_SET(STDIN_FILENO, allfds);
	FD_SET(me.getfd(), allfds);
	//maxfd=STDIN_FILENO;
	//if (me.getfd() > maxfd) maxfd=me.getfd();
	maxfd=me.getfd();
	
	struct timeval timeout;
		timeout.tv_sec=ALLUPDATE_INTERVAL;
		timeout.tv_usec=0;

	//
	// MAIN LOOP
	//
	int noready;
	while ( ! wantexit ) {
		memcpy(readfds, allfds, sizeof(fd_set));
		// SELECT CALL
		noready = select(maxfd+1, readfds, writefds, errorfds, &timeout);
		// WARNING: using the returnvalue of timeout is not portable to
		//          other systems than Linux!

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
		} else if ( noready== 0 ) {	///// TIMEOUT
			timeout.tv_sec = ALLUPDATE_INTERVAL;
			timeout.tv_usec = 0;
			printstati();
		} else {// if noready > 0)	///// DATA
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
				run_command_packet(&rxpacket);
			}
		} // end if noready > 0
	} // end main loop
	
	eprintf("subscript terminating.\n");
	return 0;
}
