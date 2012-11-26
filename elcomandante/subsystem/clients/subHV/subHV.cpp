#include "selectable_sclient.h"
#include <error.h>
#include <unistd.h>
// _GNU_SOURCE for char* basename(char*) definition
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>
#include <stdlib.h>
#include <daemon.h>
#include <time.h>

// GLOBALS
int wantexit = 0;
sclient_selectable* me = NULL;

// GLOBAL command-line flags
struct cmdflags_t {
	unsigned int BA;	///< VME BaseAddress
	char* abo_basename;	///< base name for channel status
	char* status_abo;	///< name of the status abo
	int status_interval;	///< time in seconds between status packet sends
	char* clientid;
} cmdflags;

// cmdflags default values (set in parse_cmdline)
#define DEFAULT_BASEADDRESS	0x4000
#define DEFAULT_ABO_BASENAME	"/test/HV"
#define DEFAULT_STATUS_ABO	"/test/HV/status"
#define DEFAULT_STATUS_INTERVAL	5
#define DEFAULT_CLIENTID	basename(argv[0])


// from parser
int yyparse(void);
extern FILE* yyin;


void help_and_exit() {
	printf("\n");
	printf("Syntax:\n");
	printf("        subHV [-h|--help] [-i|--interval <sec>]\n");
	printf("              [-ba|--baseaddress <BA>] [-n|--name <abo>]\n");
	printf("              [-id <clientid>]\n");
	printf("\n");
/*	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");*/
	fflush(stdout);
	exit(0);
}


#define NEXT_ARG	(*argc)--; argv++
int parse_cmdline(int* argc, char** argv) {

	// set default cmdflags:
	cmdflags.BA = DEFAULT_BASEADDRESS;
	cmdflags.abo_basename = DEFAULT_ABO_BASENAME;
	cmdflags.status_interval = DEFAULT_STATUS_INTERVAL;
	cmdflags.status_abo = DEFAULT_STATUS_ABO;
	cmdflags.clientid = DEFAULT_CLIENTID;

	NEXT_ARG;
	while (*argc > 0) {
		if ( strcmp(argv[0], "--baseaddress") == 0 ||
		     strcmp(argv[0], "-ba") == 0 ) {
			if ( *argc > 1 ) {
				cmdflags.BA = strtol(argv[1], NULL, 0);
				NEXT_ARG;
			} else { eprintf("ERROR: --baseaddress requires a numerical argument!\n"); return -1; }
		}
		else if ( strcmp(argv[0], "--name") == 0 ||
		     strcmp(argv[0], "-n") == 0 ) {
			if ( *argc > 1 ) {
				cmdflags.abo_basename = strdup(argv[1]);
				NEXT_ARG;
			} else { eprintf("ERROR: --name requires an abo-name argument!\n"); return -1; }
		}
		else if ( strcmp(argv[0], "--interval") == 0 ||
		     strcmp(argv[0], "-i") == 0 ) {
			if ( *argc > 1 ) {
				cmdflags.status_interval = strtol(argv[1], NULL, 0);
				NEXT_ARG;
			} else { eprintf("ERROR: --name requires an abo-name argument!\n"); return -1; }
		}
		else if ( strcmp(argv[0], "--clientid") == 0 ||
		     strcmp(argv[0], "-id") == 0 ) {
			if ( *argc > 1 ) {
				cmdflags.clientid = strdup(argv[1]);
				NEXT_ARG;
			} else { eprintf("ERROR: --clientid requires an argument!\n"); return -1; }
		}
		else if ( strcmp(argv[0], "--help") == 0 ||
		     strcmp(argv[0], "-h") == 0 ) {
			help_and_exit();
		}
		else {
			printf("UNKNOWN ARGUMENT:\n");
			printf("argc: %d\n", *argc);
			printf("argument: -%s-\n", argv[0]);
			return -1;
		}
//		if ( strcmp(argv[0], "-d") == 0 ) {
//			cmdflags.daemonize++;
//		}
		NEXT_ARG;
	}
	return 0;
}

int parsepacket(packet_t* packet) {
	int fd[2];
	if(pipe(fd)<0){
		eperror("Couldn't open pipe");
		return -1;
	}
	if(yyin!=NULL){ //FIXME
		fclose(yyin);
	}
	yyin = fdopen(fd[0],"r");
	if(write(fd[1], packet->data(), packet->datalen())!=packet->datalen()){
		eperror("Couldn't write packet to pipe");
		return -1;
	}
	close(fd[1]);
	return yyparse();
} 

#ifndef MAX_NAMELEN
#define MAX_NAMELEN	256
#warning Definition of MAX_NAMELEN must be equal to that in abo.h!
#endif
void do_status_update(int sig) {
	alarm(cmdflags.status_interval);
	if (me==NULL) return;
	me->printf("%ld sending status info\n", time(NULL));
	char aboname[MAX_NAMELEN];
	for (int i=0; i<4; i++) {
		snprintf(aboname, MAX_NAMELEN, "%s/channel_%02d", cmdflags.abo_basename, i);
		double Vset = 1234.00;	// [V]    from hv.getSetVoltage() FIXME
		double Vmon = 1233.97;
		double Iset = .0;	// [uA]   from hv.GetImon(); FIXME
		double Imon = 50.;
		char mode = 'U';
		me->aprintf(aboname, "%ld ON %c %lfVmon %lfVset %lfuAmon %lfuAset\n",
				time(NULL), mode, Vmon, Vset, Imon, Iset);
	}
}

void clean_exit(int sig) {
	eprintf("clean exit\n");
	if (wantexit > 5) abort();
	wantexit++;
}

int main(int argc, char* argv[]) {

	if ( parse_cmdline(&argc, argv) < 0 ) help_and_exit();
	// FIXME: check cmdflags consistency here!
	// status_interval > 0
	// abo_basename ends with "/" and has no space
	// no space in clientid
	// BA >> 0,  0xFF & BA != 0
	signal_handler(SIGINT, clean_exit);	
	signal_handler(SIGTERM, clean_exit);	

/*	printf("\nVME:\n");
	printf("\tbase address: 0x%06X\n", cmdflags.BA);
	printf("SCLIENT:\n");
	printf("\tclientid:     %s\n", cmdflags.clientid);
	printf("\tabo_basename: %s\n", cmdflags.abo_basename);
	printf("\tinterval:     %d sec\n", cmdflags.status_interval);*/

	me = new sclient_selectable();
	if (!me->isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}
	me->setid(cmdflags.clientid);
//	me->subscribe(cmdflags.status_abo); not needed!
	me->setDefaultSendname(cmdflags.status_abo);

	signal_handler(SIGALRM, do_status_update);
	do_status_update(0);

	packet_t packet;

	wantexit=0;
	int selret=0;
	int ret=0;
	while (!wantexit) {
		me->printf("waiting...\n");
		switch ( selret = selectable::run() ) {
		case -1:
			if ( errno == EINTR ) continue;
			eperror("select() call failed");
			wantexit++;
			break;
		case 0:
			// timeout (does not occour)
			break;
		default:
			me->printf("received packet\n");
			if ( me->isready(CHK_READ) ) {
				me->recvpacket(packet);
				if ( !packet.isValid() ) {
					me->printf("received invalid packet\n");
					break;
				}
				if ( packet.type != PKT_SETDATA ) {
					me->printf("discarding non-PKT_SETDATA packet\n");
					break;
				}
				if ( (ret=parsepacket(&packet)) != 0) {
					me->printf("could not parse packet: '%.*s'(len%d)! Return: %d\n", packet.datalen(), packet.data(), packet.datalen(),ret);
				}
				selret--;
			}
			if (selret != 0) { me->printf("selret = %d not zero!\n", selret); wantexit++; }
		}
	}
	me->printf("subHV terminating.\n");
	if (me != NULL) { delete me; me=NULL; }
	return 0;
} 
