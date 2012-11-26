#include <string.h>	// strrchr, strncpy

#include "child_info_t.h"

child_info_t::child_info_t(int argc, char* argv[], char* ClientID) {
	// set default names, if none given
	if ( ClientID == NULL ) {
		char *ptr;
		char hostname[MAX_IDLEN];
		char command[MAX_IDLEN];

		ptr = strrchr(argv[0], '/');	// find last '/'
		strncpy(command, ( ptr==NULL ? argv[0] : ptr+1 ), MAX_IDLEN);
		command[MAX_IDLEN-1]=0;
		
		if (gethostname(hostname, MAX_IDLEN) <0 ) hostname[MAX_IDLEN-5]=0; // ensure term \0
		if ((ptr=strchr(hostname,'.'))!= NULL) *ptr=0; // cut domain part

		snprintf(clientid, MAX_IDLEN, "%s:%s", hostname, command);
		clientid[MAX_IDLEN-1]=0;
	} else {
		strncpy(clientid, ClientID, MAX_IDLEN);
		clientid[MAX_IDLEN-1]=0;
	}
	snprintf(inabo,  MAX_NAMELEN, "%s/%s/%s", DEFAULT_IN_PREF,  clientid, DEFAULT_IN_SUFF);
	snprintf(outabo, MAX_NAMELEN, "%s/%s/%s", DEFAULT_OUT_PREF, clientid, DEFAULT_OUT_SUFF);
	snprintf(errabo, MAX_NAMELEN, "%s/%s/%s", DEFAULT_ERR_PREF, clientid, DEFAULT_ERR_SUFF);
	
	mychild = new child_t(argc, argv, clientid);
}

child_info_t::~child_info_t() {
	delete mychild;
}

child_t *child_info_t::Child() {
	return mychild;
}

int child_info_t::setClientID(char* to) {
	if ( strncpy(clientid, to, MAX_IDLEN) == NULL )
		{ inabo[MAX_IDLEN-1]=0; return -1; }
	return 0;
}
int child_info_t::setInabo(char* to) {
	if ( strncpy(inabo, to, MAX_NAMELEN) == NULL )
		{ inabo[MAX_NAMELEN-1]=0; return -1; }
	return 0;
}
int child_info_t::setOutabo(char* to) {
	if ( strncpy(outabo, to, MAX_NAMELEN) == NULL )
		{ inabo[MAX_NAMELEN-1]=0; return -1; }
	return 0;
}
int child_info_t::setErrabo(char* to) {
	if ( strncpy(errabo, to, MAX_NAMELEN) == NULL )
		{ inabo[MAX_NAMELEN-1]=0; return -1; }
	return 0;
}

const char* child_info_t::ClientID() const	{ return clientid; }
const char* child_info_t::Inabo() const		{ return inabo; }
const char* child_info_t::Outabo() const	{ return outabo; }
const char* child_info_t::Errabo() const	{ return errabo; }

void child_info_t::PrintInfo() {
	if ( mychild != NULL ) mychild->PrintInfo();
	printf("\t<inabo>:     %s\n", inabo);
	printf("\t<outabo>:    %s\n", outabo);
	printf("\t<errabo>:    %s\n", errabo);
	printf("\t<clientid>:  %s\n", clientid);
	fflush(stdout);
}
