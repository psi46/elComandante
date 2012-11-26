/*
 * logfile_t.cpp
 *
 * Dennis Terhorst
 */
#include "logfile_t.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdarg.h>

#include <string.h>
#include <time.h>

//#include "error.h"

#include <stdio.h>
#include <errno.h>
#define eprintf	printf
#define eperror printf

char logfile_t::LOGDIR[MAX_LOG_NAMELEN] = DEFAULT_LOGDIR;

logfile_t::logfile_t(const char* Name) {
	strncpy(LOGDIR, DEFAULT_LOGDIR, MAX_LOG_NAMELEN);

	bzero(name, MAX_LOG_NAMELEN);
	strncpy(name, LOGDIR, MAX_LOG_NAMELEN);
	strncat(name, Name, MAX_LOG_NAMELEN-strlen(name)-1);

	int i;
	while ((i=strspn(name, FILENAME_GOODCHARS)) < strlen(name) ) {
		name[i] = FILENAME_BADCHARREPLACE;
	}
	fd = -1;
	last = -1;
}
logfile_t::logfile_t(const logfile_t& copy) {
	*this = copy;	// FIXME: do not copy!
	eprintf("%s:%d: critical instance copy\n",__FILE__,__LINE__);
	//copy.fd = -1;	// FIXME: invalidate old LOG object (cause it not to close the file!)
}

logfile_t::~logfile_t() {
	closefile();
//		else {
//			eprintf("%s:%d: instance delete without file close!\n",__FILE__,__LINE__);
//		}
}

int logfile_t::closefile() {
	if (fd>=0) {
		eprintf("closing logfile \'%s\'\n", name);
		if ( fsync(fd) < 0 )
			eperror("%s: ERROR: could not sync logfile on close", __FILE__);
		if ( close(fd) < 0 )
			eperror("%s: ERROR: could not close logfile properly", __FILE__);
	}
}

bool logfile_t::isOK() {
	return (fd>=0);
}

int logfile_t::openfile() {
	char namecopy[MAX_LOG_NAMELEN];
	strcpy(namecopy, name);
	char buffer[MAX_LOG_NAMELEN];
	char* filename = rindex(namecopy, '/')+1;
	char* dir;
	char* saveptr=NULL;
	
	bzero(buffer, MAX_LOG_NAMELEN);	// clear buffer;

	// for each directory
	for (dir = strtok_r(namecopy, "/", &saveptr); dir < filename; dir = strtok_r(NULL,"/", &saveptr) )
	{
		//eprintf("dir  %s\n", dir);
		strncat(buffer, dir, MAX_LOG_NAMELEN-strlen(buffer)-1);
		errno = 0;
		if ( mkdir(buffer, LOGDIR_PERMISSIONS) < 0 ) {
			if ( errno != EEXIST ) {
				eperror("ERROR: Could not create logdir \'%s\'", buffer);
				return -1;
			} // else: dir exists.
		} else {
			eprintf("created logdir \'%s\'\n", buffer);
		}
		strncat(buffer, "/", MAX_LOG_NAMELEN-strlen(buffer)-1);
	}

	// append filename
	//eprintf("file %s\n", filename);
	char stamp[64]; int stamplen=64;
	mktimestamp(stamp, stamplen);
	strncat(buffer, filename, MAX_LOG_NAMELEN-strlen(buffer)-1);
	strncat(buffer, stamp, MAX_LOG_NAMELEN-strlen(buffer)-1);

	/* old check for correct transfer of name to buffer
	if ( strcmp(name, buffer) != 0) {
		eprintf("wanted name=\'%s\'\n", name);
		eprintf(" got buffer=\'%s\'\n", buffer);
		eprintf("ERROR: wrong logfile! (see %s:%d)\n", __FILE__,__LINE__);
		// this should actually never happen, but really bad things
		// will happen if the opened file and stored filename are out
		// of sync.
	}*/

	errno = 0;
	fd = open(buffer, O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND, LOGFILE_PERMISSIONS);
	if (errno != 0) {
		eperror("ERROR: Could not open logfile \'%s\'", name);
		return fd;
	}
	eprintf("opened logfile \'%s\' as fd %d\n", buffer, fd);
	return fd;
}

logfile_t& logfile_t::operator= (const logfile_t& other) {
	strcpy(name, other.name);
	fd = other.fd;
	return *this;
}

bool logfile_t::operator== (const logfile_t& other) const {
	return (strcmp(name, other.name) == 0);
}

int logfile_t::print(const char* format, ...) {
	if ( wantnewfile() ) this->reopen();

	va_list va;
	va_start(va, format);
	static char buffer[1024];
	int len=-1;
	len=vsnprintf(buffer, 1024, format, va);

	if (fd >= 0) {
		if ( write(fd, buffer, len) < 0 ) {
			eperror("%s:%d: ERROR: could not write() to logfile %s!\n", __FILE__, __LINE__, name);
			len = -1;
		}
		fsync(fd);
	}
	va_end(va);
	return len;
}

/**
 * set the class wide logdir path
 */
int logfile_t::setLogdirectory(char* logdir) {
	strncpy(LOGDIR, logdir, MAX_LOG_NAMELEN);
//	classflags |= LOGFILE_CF_NEEDREOPEN;
	return 0;
}

const char* logfile_t::getLogdirectory() {
	return LOGDIR;
}
/**
 * marker for log-rotation
 *
 * This function returns non-zero if the logfile needs to be reopened and is called
 * from the print() function before each disk write.
 * You can control the time induced reopening with the \ref LOGFILE_ROTATION
 * define at compile time.
 *
 * \note \li The reopen for log rotation is done on next write, so there
 * might not be a file for each logging interval. \li Also note that the time
 * used for rotation is subservers localtime, so packets arriving after
 * a logrotate will be logged into the new file, regardless of an older
 * timestamp they may have.
 */
int logfile_t::wantnewfile() {
	struct tm now;
	time_t nowsec;

	time(&nowsec);
	localtime_r(&nowsec, &now);

	if ( now.LOGFILE_ROTATION != last ) {
		last = now.LOGFILE_ROTATION;
		return 1;		// want reopen due to logrotation
	}

//	if ( classflags & LOGFILE_CF_NEEDREOPEN )
//		return 1;		// want reopen due to flag
	
	return 0;	
}

void logfile_t::mktimestamp(char* buffer, int buflen) {
	struct tm now;
	time_t nowsec;

	time(&nowsec);
	localtime_r(&nowsec, &now);

	snprintf(buffer, buflen, "_%04d%02d%02d.%02d%02d%02d",
		now.tm_year+1900, now.tm_mon+1, now.tm_mday,
		now.tm_hour, now.tm_min, now.tm_sec);
	buffer[buflen-1]=0;

	return;
}

int logfile_t::reopen() {
	int ret;
	ret=closefile();
	if ( ret < 0 )
		return ret;
	ret = openfile();
	return ret;
}
