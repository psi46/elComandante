#ifndef PIPE_H
#define PIPE_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// informational abo from parent
#define SCRIPT_ABO "/process/stati"

// common stderr redirect
#define DEFAULT_ERR "/process/errors"

// Send a status update packet each n seconds (status changes
// are nevertheless always reported immediately)
#define STATUS_INTERVAL_SEC	5

#ifndef MAX_NAMELEN
#warning MAX_NAMELEN redefinition must comply with abo.h
#define MAX_NAMELEN 256
#endif
#ifndef MAX_IDLEN
#warning MAX_IDLEN redefinition must comply with clientinfo.h
#define MAX_IDLEN 256
#endif


void help() {
	printf("\nSyntax: pipe\t[-d] [-v] [-h|--help] [-r <type>|--restart <type>]\n\
		[-i <inabo>] [-o <outabo>] [-e <errabo>]\n\
		[-s <scriptabo>|--status <scriptabo>]\n\
		[-id <clientid>] <command> [<args> ...]\n\
\n\
Description:\n\
	Starts the given <command> as a child process of a subsystem client\n\
	connection after redirecting the processes stdio file descriptors to\n\
	subsystem abos.\n\
\n\
	-d	start processes as background process (daemon). This will only\n\
		work if <command> is given as an absolute path name, because\n\
		the daemon parent will chdir to /.\n\
\n\
	-h, --help\n\
		print this help message.\n\
\n\
	-r <type>, --restart <type>\n\
		restart the child process if it died of the condition given\n\
		by <type>. Where <type> is one of\n\
			1	if child terminated\n\
			2	if killed by a signal\n\
			3	if killed or terminated\n\
\n\
	-i <abo>, -o <abo>, -e <abo>\n\
		set specified <abo> as stdin, stdout or stderr redirection\n\
		for the child process.\n\
\n\
	-s <scriptabo>, --status <scriptabo>\n\
		send status info to the given channel <scriptabo>, instead\n\
		of the default (see below).\n\
\n\
	-id <clientid>\n\
		register as client <clientid> to the server, instead of\n\
		the default id (see below).\n\
\n\
	-c	use common stderr redirection abo (see below).\n\
	\n");
	printf("Default Settings:\n");
//	printf("\t<inabo>:     %s<host:proc>/stdin\n",  DEFAULT_IN_PREF);
//	printf("\t<outabo>:    %s<host:proc>/stdout\n", DEFAULT_OUT_PREF);
//	printf("\t<errabo>:    %s<host:proc>/stderr\n", DEFAULT_ERR_PREF);
	printf("\t<common>:    %s\n", DEFAULT_ERR);
	printf("\t<scriptabo>: %s\n", SCRIPT_ABO);
	printf("\t<clientid>:  %s\n", "<command>");
//	printf("\n\tWhere <host:proc> is replaced by the local hostname, a colon\n\
//	and the <command> string.\n\n");
	fflush(stdout);
	exit(0);
}


// Mask values for cmdflags_t::restart
#define RESTART_TERM	1
#define RESTART_KILLED	2

// command line (parent) parameters
struct cmdflags_t {
	int daemonize;
	int verbose;
//	int commonerr;
	int restart;
	char scriptabo[MAX_NAMELEN];
	char	inabo[MAX_NAMELEN];  // MAX_NAMELEN defined in sclient.h <- abo.h
	char	outabo[MAX_NAMELEN];
	char	errabo[MAX_NAMELEN];
	char	clientid[MAX_IDLEN]; // MAX_NAMELEN defined in sclient.h <- clientinfo.h
	char**	rest_argv;
	int	rest_argc;
};

int parse_commandline(int argc, char **argv, struct cmdflags_t& cmdflags) {
	//
	// parse parameters
	// 
	printf("argc:%d, argv[0]=\"%s\"\n", argc, argv[0]);
	while ( ++argv, --argc>0 ) {	// shift
		printf("argc:%d, argv[0]=\"%s\"\n", argc, argv[0]);
		if ( strcmp(argv[0], "-d")==0 ) { cmdflags.daemonize++; continue; }
		//if ( strcmp(argv[0], "-c")==0 ) { cmdflags.commonerr++; continue; }
		if ( strcmp(argv[0], "-v")==0 ) { cmdflags.verbose++; continue; }
		if ( strcmp(argv[0], "-r")==0 || strcmp(argv[0], "--restart")==0) {
			if (argc == 1) {
				printf("ERROR: {-r|--restart} needs an argument! See --help.\n");
				return -1;
			}
			cmdflags.restart = atoi(argv[1]);
			++argv, --argc;
			continue;
		}
		if ( strcmp(argv[0], "-i")==0 ) {
			if (argc == 1) {
				printf("ERROR: -i needs an argument! See --help.\n");
				return -1;
			}
			if (argv[1][0] == '-') {
				printf("Warning: Stdin abo name starts with '-'. Did you forget the parameter?\n");
			}
			strncpy(cmdflags.inabo, argv[1], MAX_NAMELEN);
			++argv, --argc;
			continue;
		}
		if ( strcmp(argv[0], "-o")==0 ) {
			if (argc == 1) {
				printf("ERROR: -o needs an argument! See --help.\n");
				return -1;
			}
			if (argv[1][0] == '-') {
				printf("Warning: Stdout abo name starts with '-'. Did you forget the parameter?\n");
			}
			strncpy(cmdflags.outabo, argv[1], MAX_NAMELEN);
			++argv, --argc;
			continue;
		}
		if ( strcmp(argv[0], "-e")==0 ) {
			if (argc == 1) {
				printf("ERROR: -i needs an argument! See --help.\n");
				return -1;
			}
			if (argv[1][0] == '-') {
				printf("Warning: Stderr abo name starts with '-'. Did you forget the parameter?\n");
			}
			strncpy(cmdflags.errabo, argv[1], MAX_NAMELEN);
			++argv, --argc;
			continue;
		}
		if ( strcmp(argv[0], "-c")==0 ) {
			printf("switching to common stderr: %s\n", DEFAULT_ERR);
			snprintf(cmdflags.errabo, MAX_NAMELEN, "%s", DEFAULT_ERR);
			continue;
		}
		if ( strcmp(argv[0], "-s")==0 || strcmp(argv[0], "--status")==0 ) {
			if (argc == 1) {
				printf("ERROR: -s needs an argument! See --help.\n");
				return -1;
			}
			if (argv[1][0] == '-') {
				printf("Warning: Status abo name starts with '-'. Did you forget the parameter?\n");
			}
			strncpy(cmdflags.scriptabo, argv[1], MAX_NAMELEN);
			++argv, --argc;
			continue;
		}
		if ( strcmp(argv[0], "-id")==0 ) {
			if (argc == 1) {
				printf("ERROR: -id needs an argument! See --help.\n");
				return -1;
			}
			if (argv[1][0] == '-') {
				printf("Warning: Client name starts with '-'. Did you forget the parameter?\n");
			}
			strncpy(cmdflags.clientid, argv[1], MAX_IDLEN);
			++argv, --argc;
			continue;
		}

		if ( strcmp(argv[0], "-h")==0 ) { help(); }
		if ( strcmp(argv[0], "--help")==0 ) { help(); }
		cmdflags.rest_argv = argv;
		cmdflags.rest_argc = argc;
		break;
	}


	if (cmdflags.scriptabo[0] == 0) {				// use default abo for status if none given
		strncpy(cmdflags.scriptabo, SCRIPT_ABO, MAX_NAMELEN);
	}
	return 0;
}
#endif //ndef PIPE_H
