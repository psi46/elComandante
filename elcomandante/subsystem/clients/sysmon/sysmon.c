/**
 * \file sysmon.c
 * \author Dennis Terhorst
 * \date March 2009
 */
#include <stdio.h>
#include <time.h>
#include <sys/types.h>	// open
#include <sys/stat.h>	// open
#include <fcntl.h>	// open
#include <unistd.h>	// close
#include <stdlib.h>	// EXIT_SUCCESS
#include <string.h>	// strchr
#include <sys/select.h>
#include <errno.h>

#include "../../sclient.h"
#include "../../daemon.h"

// maximum values (FIXME: should be from limits.h)
#define MAX_MONITORS	16
#define MAX_FNAMELEN	512

// log time interval
#define MONITOR_INTERVAL	10

// file list to load
#define CONFIG_FILE	"monitor_files"

// clientid construction
#define SYSMON_SUFFIX	":sysmon"

// abo name construction
//#define ABO_PREFIX	"/health/"
//#define ABO_NAME	"%s%s", ABO_PREFIX, hostname
#define ABO_NAME	"/system/health"

#define MAX_LINELEN	MAX_PACKETLEN
// string concatenation macro
#define sncatf(st, length, format, args...) snprintf(&(st[strlen(st)]), length-strlen(st), format, ##args )

volatile int wantexit = 0;
int read_monitor_file_list(char* cfgfile, char list[MAX_MONITORS][MAX_FNAMELEN], int* nof);

void handler(int sig) {
	wantexit++;
}

struct cmdflags_t {
	int daemonize;
};

struct cmdflags_t cmdflags = {0};

int main(int argc, char *argv[]) {

	char hostname[MAX_FNAMELEN]={0};
	char file[MAX_MONITORS][MAX_FNAMELEN];
	int nof=0;
	FILE* fd[MAX_MONITORS];
	char line[MAX_LINELEN];
	char buffer[MAX_LINELEN];
	char* ptr;

	for (int i=0; i<argc; i++) {
		if ( strcmp("-d", argv[i]) == 0 ) cmdflags.daemonize++;
	}
	//
	// read config
	//
	read_monitor_file_list(CONFIG_FILE, file, &nof);
	printf("monitoring %d files:\n", nof);
	for (int i=0; i<nof; i++) { printf("- %s\n", file[i]); }

	if ( nof < 1 ) {
		fprintf(stderr, "no files to monitor...\n");
		exit(EXIT_FAILURE);
	}

	//
	// SETUP SERVER CONNECTION
	//
	signal_handler(SIGINT, handler);
	signal_handler(SIGTERM,handler);

	sclient me;
	if ( !me.isOK() ) {
		printf("could not initialize subsystem client\n");
		exit(EXIT_FAILURE);
	}

	if ( cmdflags.daemonize ) daemonize_me();

	// build clientid
	gethostname(hostname, MAX_FNAMELEN);
	if ( (ptr=strchr(hostname, '.')) != NULL ) { *ptr=0; }

	sprintf(buffer, "%s%s", hostname, SYSMON_SUFFIX);
	printf("setting id to %s\n", buffer);
	me.setid(buffer);

	sprintf(buffer, ABO_NAME);
	me.setDefaultSendname(buffer);


	//printf("going to background...\n"); fflush(stdout);
	//daemonize_me();

	//
	// MAIN LOOP
	//
	while(!wantexit) {

		line[0]=0;
		for (int i=0; i<nof; i++) {
			fd[i] = fopen(file[i], "r");	// OPEN
			if ( fd[i] == NULL ) {
				fprintf(stderr, "could not open file %s", file[i]); perror("");
				exit(EXIT_FAILURE);
			}
			fgets(buffer, MAX_LINELEN, fd[i]); // READ
			if ( (ptr=strchr(buffer, 10))!=NULL ) *ptr=0;
			if ( (ptr=strchr(buffer, 13))!=NULL ) *ptr=0;
			sncatf(line, MAX_LINELEN, "%s%s=%s", (i>0?", ":""), strrchr(file[i], '/')+1, buffer); fflush(stdout);
			fclose(fd[i]);			// CLOSE
		}
		me.printf("%ld %s > %s\n", time(NULL), hostname, line);
		sleep(MONITOR_INTERVAL);		// SLEEP

	} // end main loop

	fprintf(stderr, "clean exit.\n");
	return EXIT_SUCCESS;
}


/*
 * read cfgfile into the list array
 */
int read_monitor_file_list(char* cfgfile, char list[MAX_MONITORS][MAX_FNAMELEN], int* nof) {

	FILE* listfile = fopen(cfgfile, "r");
	if (listfile == NULL) {
		perror("could not open config file");
		return EXIT_FAILURE;
	}
	for (int i=0; i<MAX_MONITORS; i++) {
		if ( ferror(listfile) ) {
			perror("config file read error");
			return EXIT_FAILURE;
		}
		if ( feof(listfile) ) {
			break;
		}
		fgets(list[*nof], MAX_FNAMELEN, listfile);
		if ( list[*nof][0]==0 ) continue;
		if ( list[*nof][0]==13 ) continue;
		if ( list[*nof][0]==10 ) continue;
		if ( list[*nof][0]=='#' ) continue;
		char* ptr;
		if ( (ptr=strchr(list[*nof], 10)) != NULL ) *ptr=0;	// cut trailing CR
		if ( (ptr=strchr(list[*nof], 13)) != NULL ) *ptr=0;	// cut trailing NL
		(*nof)++;
	}
	return fclose(listfile);

}

