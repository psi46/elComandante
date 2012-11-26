/*
 * subscript.cpp
 * subserver client to dynamically start and stop scripts
 * Dennis Terhorst
 * 13 Mar 2008
 */
//#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h> // sleep

#include <stdio.h>	// fopen
//#include <sys/select.h>	// select()
#include "../../error.h"

// for directory listings
#include <sys/types.h>		// opendir, stat
#include <dirent.h>		// opendir
#include <unistd.h>		// chdir
#include <errno.h>
#include <sys/stat.h>		// stat
#include <vector>
#include <string>
//#include <iostream>

//#include "sclient.h"
//#include "script_t.h"
#include "../../packet_t.h"
#include "../../slist.h"
#include "../../command_t.h"


#define	MAX_LINELEN	512
#define MAX_ARGS	16

#define MAX_NAMELEN	MAX_PACKETLEN
#define PROTOCOL_DIR	"./protocols"

#define SUBSCRIPT_CMDFUNC       int (*)(int argc, char* argv[])

using namespace std;

// Script Flags
#define SF_FORCESTATUS	1
#define SF_RUNONCE	2
#define SF_IGNOREERROR	4
class script_info {
public:
	char	name[MAX_NAMELEN];
	char	status;
	int	flags;

	script_info() {
		name[0]=0;
		status=0;
		flags=0;
	}

	bool operator==(const script_info& other) const {
		return (strcmp(name, other.name)==0);
	}

	int setName(const char* pname) {
		strncpy(name, pname, MAX_NAMELEN);
		return 0;
	}

	int setStatus(const char s) { // char
		switch (s) {
		case 'K':
		case 'S':
			status = s;
			return 0;
		}
		return -1;
	}
	
	int setStatus(const char* s) {	// first char of char*
		if (strlen(s) > 1) eprintf("WARNING: trailing chars in script_info::setStatus()\n");
		return setStatus(s[0]);
	}

	bool complies(script_info to) {
		return (status == to.status);
	}
};

/*class script_info {
public:
	char	name[MAX_NAMELEN];
	char	soll;
	char	ist;
	void setName(const char* pname) {
		strncpy(name, pname, MAX_NAMELEN-1);
		name[MAX_NAMELEN-1]=0;
	}
	bool operator==(script_info& other) const {
		return (strcmp(name, other.name)==0);
	}
};*/


class protocol_t {
public:
	char	name[MAX_NAMELEN];
	slist<script_info>	state;

	protocol_t() {
		name[0]=0;

	}

	protocol_t(const protocol_t& copy) {
		strncpy(name, copy.name, MAX_NAMELEN);
		state = copy.state;
	}

	bool operator==(const protocol_t& other) const {
		return (strcmp(name, other.name)==0);
	}

	void setName(const char* pname) {
		strncpy(name, pname, MAX_NAMELEN-1);
		name[MAX_NAMELEN-1]=0;
	}

	int loadfile(const char* filename) {
		FILE* p;

		p = fopen("protocols/all_off","r");
		if ( p == NULL ) {
			eperror("could not open protocol file");
			return 1;
		}

		char buffer[MAX_LINELEN];
		int argc = MAX_ARGS;
		char *argv[MAX_ARGS];
		int l=0;
		while ( ! (feof(p) || ferror(p)) ) {
			if ( fgets(buffer, MAX_LINELEN, p) == NULL ) {
				break;
			}
			l++;
			if (buffer[0] == '#') continue;	// discard comments;
			eprintf("line %d:\n", l);
			command_t<SUBSCRIPT_CMDFUNC>::parse(buffer, strlen(buffer), argc, argv);
			if ( argc == 0 ) continue;
			if ( argc != 2 ) {
				eprintf("wrong number (%d) of args in line %d length=%d\n", argc, l, strlen(argv[0]));
				continue;
			}
			
			script_info s;
			s.setName(argv[0]);
			s.setStatus(argv[1]);
			FUNCTIONTRACKER;
			//s.setFlags(argv[2]);
			state.add(s);
			eprintf("state:\n");
			for (int i=0; i<argc; i++) {
				eprintf("	argv[%d]=\"%s\"\n", i, argv[i]);
			}
		}

		if ( fclose(p) != 0 ) {
			eperror("could not close protocol file");
			return 1;
		}
	}

#define P_OBJECTS	-1
#define P_CONCURS	1
#define P_DONTCARE	0
	int opinion(script_info s) {
		script_info* soll = state.get(s);
		if ( soll == NULL )		return P_DONTCARE;
		if ( s.complies(*soll) )	return P_CONCURS;
		return P_OBJECTS;
	}

};

/*class protocol_t {
public:
	char	name[MAX_NAMELEN];
	slist<script_info>	state;
	void setName(const char* pname) {
		strncpy(name, pname, MAX_NAMELEN-1);
		name[MAX_NAMELEN-1]=0;
	}
	bool operator==(protocol_t& other) const {
		return (strcmp(name, other.name)==0);
	}
};*/

slist<protocol_t> protocol;

int getdir (string dir, vector<string> &files)
{
	DIR *dp;
	struct dirent *dirp;
	if((dp  = opendir(dir.c_str())) == NULL) {
		eprintf("Error(%d) opening %s\n", errno, dir.c_str());
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		files.push_back(string(dirp->d_name));
	}
	closedir(dp);
	return 0;
}

int load_protocol(const char* filename) {
	FILE* fp;
	struct stat fileinfo;
	script_info sinfo;
	protocol_t proto;

	
	proto.setName(filename);
	
	if ( stat(filename, &fileinfo) < 0 ) {
		eperror("could not stat \"%s\"", filename);
		return 1;
	}
	
	if ( ! S_ISREG(fileinfo.st_mode) ) {
		//eprintf("%s is not a regular file!", filename);
		return 1;
	}
	
	fp = fopen(filename,"r");
	if ( fp == NULL ) {
		eperror("could not open protocol file %s!", filename);
		return 1;
	}

	eprintf("loading protocol %s\n", filename);

	char buffer[MAX_LINELEN];
	int argc = MAX_ARGS;
	char *argv[MAX_ARGS];
	int l=0;
	while ( ! (feof(fp) || ferror(fp)) ) {
		buffer[0] = 0;
		fgets(buffer, MAX_LINELEN, fp); l++;
		if (buffer[0] == 0) continue;	// no read
		if (buffer[0] == '#') continue;	// discard comments;
		command_t<SUBSCRIPT_CMDFUNC>::parse(buffer, strlen(buffer), argc, argv);
		if ( argc == 0 ) continue;	// empty line
		if ( argc != 2 ) {
			eprintf("wrong number (%d) of args in line %d\n", argc, l);
			continue;
		}
		if (sinfo.setName(argv[0]) < 0 ) {
			eprintf("%s:%d: invalid scriptname '%s'\n", filename, l, argv[0]);
			continue;
		}
		if (sinfo.setStatus(argv[1][0]) < 0) {
			eprintf("%s:%d: invalid script status '%s'\n", filename, l, argv[1][0]);
			continue;
		}
		FUNCTIONTRACKER;	
		proto.state.add(sinfo);		// add scriptstatus to protocol definition
	}
	
	protocol += proto;	// ADD TO GLOBAL LIST
	
	if ( fclose(fp) != 0 ) {
		eperror("could not close protocol file");
		return 1;
	}
	return 0;
}

int nullfunc(int argc, char* argv[]) {
	return 0;
}


int main(void) {
	string dir = string(PROTOCOL_DIR);
	vector<string> files = vector<string>();


	if ( chdir(PROTOCOL_DIR) < 0 ) {
		eperror("could not chdir to %s", PROTOCOL_DIR);
		return 1;
	}
	
	getdir(".",files);

	for (unsigned int i = 0;i < files.size();i++) {
		load_protocol(files[i].c_str());
	}
	eprintf("protocols loaded:\n");

	for (int p=0; p<protocol.length(); p++) {
		eprintf("%s:\n", protocol[p].name);
		for (int s=0; s<protocol[p].state.length(); s++) {
			eprintf("\t%c  %s\n", protocol[p].state[s].status, protocol[p].state[s].name );
		}
	}
	/*slist<script_info> status;

	slist<protocol_t> protocol;

	protocol_t lproto;
	lproto.loadfile("protocol/all_off");
	protocol.add(lproto);

	// check all script stati
	int opinion;
	for (int s=0; s<status.length(); s++) {
		for (int p=0; p<protocol.length(); p++) {
			opinion = protocol[p].opinion(status[s]);
			if (opinion == P_OBJECTS) {
				eprintf("%s objects to script %s status!\n");
			}
			if ( opinion != P_DONTCARE ) break;
		}
	}*/

	return 0;
}
/*
#define ALLUPDATE_INTERVAL	15

#define SUBSCRIPT_CMDFUNC	int (*)(int argc, char* argv[])
sclient* meptr;
slist<script_t> scriptlist;	// list of scripts
slist<command_t<SUBSCRIPT_CMDFUNC> > cmd;	// list of commands


void printstati() {
	static bool havehead=false;
	if (!havehead)	eprintf("TIME       %s\n",	script_t::Status_head_string() );
	havehead=true;
	for (int i=0; i<scriptlist.length(); i++) {
		eprintf("%ld %s\n", time(NULL), scriptlist[i].Status_string());
	}
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
	eprintf("%ld %s\n", time(NULL), s->Status_string());	// sendpacket();
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
//	if (s->reset() < 0) {	// run script with argv[0]="run" removed;
//		eprintf("Failed to reset() %s\n\t", s->getName());
//	}
	eprintf("%ld %s\n", time(NULL), s->Status_string());	// sendpacket();
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
	eprintf("%ld %s\n", time(NULL), s->Status_string());	// sendpacket();
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
	//eprintf("received SIGCHLD:\n");
	for (int i = 0; i<scriptlist.length(); i++) {
		if ( scriptlist[i].justTerminated() ) {
			//eprintf("CHANGE:\n");
			eprintf("%ld %s\n", time(NULL), scriptlist[i].Status_string());	// sendpacket();
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
		eprintf("ERROR: could not set up signal handler for SIGINT!\n");
		exit(-1);
	}
	if ( signal(SIGCHLD, sigCHLD_handler) == SIG_ERR ) {
		eprintf("ERROR: could not set up signal handler for SIGCHLD!\n");
		exit(-1);
	}
	
	// connect sclient
	sclient me;
	if (!me.isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}
	meptr = &me;
	eprintf("subscript client initialized\n");
	
	//
	// setup scripts
	//
	scriptlist.add( script_t("truetest", "/bin/true") );
	scriptlist.add( script_t("sleeptest", "./sleepscript.sh") );
	scriptlist.add( script_t("thpc_split", "./thpc_split") );
	scriptlist.add( script_t("falsetest", "/bin/false") );
	scriptlist.add( script_t("falsetest", "/bin/false") );

	printf("--- AVAILABLE SCRIPTS ---\n");
	printstati();
	printf("-------------------------\n");

	// init controll commands
	init_commands();
	//
	// subscribe control channel
	//
	packet.setName("/subscript/control");
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	me.sendpacket(packet);
	
	me.setid("subscript");
	
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
		} else {// if noready > 0)	///// DATA*/
		/*	if ( FD_ISSET(STDIN_FILENO, readfds) ) {
				buffer[0]=0;
				fgets(buffer, BUFSIZE, stdin);
				buflen = strlen(buffer);
				if ( buffer[0] == 'x' || feof(stdin)) break;
				if ( buffer[0] == 'd' && buffer[1] == '\n' ) {	// mgm packet hack
					eprintf("Packet type set to PKT_DATA!\n");
					packet.type = PKT_DATA;
					continue;
				}
				if ( buffer[0] == 'm' && buffer[1] == '\n' ) {	// mgm packet hack
					packet.type = PKT_MANAGEMENT;
					eprintf("Packet type set to PKT_MANAGEMENT!\n");
					continue;
				}
				packet.setData(buffer, buflen);

				me.sendpacket(packet);
				
			}*/ /*
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
				printf("parsing \"%1$.*2$s\"\n", rxpacket.data(), rxpacket.datalen()-1);
				run_command_packet(&rxpacket);
			}
		} // end if noready > 0
	} // end main loop
	*/
/*	for (int i = 0; i<scriptlist.length(); i++) {
		printf("starting script[%d]...\n", i);	
		scriptlist[i].run(NULL);
	}

	eprintf("%s\n",	script_t::Status_head_string() );
	while (!scriptlist[1].isfinished()) {
		//printf("--- SCRIPT INTERMIDIATE STATUS REPORT ---\n");
		printstati();
		//printf("-----------------------------------------\n");
		sleep(1);
	}
		//printstati();
		

	printf("\n--- SCRIPT FINAL STATUS REPORT ---\n");
	printstati();
	printf("-----------------------------------------\n");*/ /*
	eprintf("program end.\n");
	return 0;
}
*/

