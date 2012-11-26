/*
 * log.h
 *
 * Dennis Terhorst
 */
#ifndef LOG_H
#define LOG_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string.h>

#include "error.h"

#include "packet_t.h"
#define MAX_LOG_NAMELEN	MAX_PACKETLEN
// give path to logs without trailing slash!
#define DEFAULT_LOGDIR	"./DATA"
#define FILENAME_GOODCHARS	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_/"
#define FILENAME_BADCHARREPLACE '_'

#define LOGDIR_PERMISSIONS	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH
#define LOGFILE_PERMISSIONS	S_IRUSR | S_IWUSR | S_IRGRP

class log {
private:
	int fd;
	char name[MAX_LOG_NAMELEN];
public:
	log(const char* Name) {
		bzero(name, MAX_LOG_NAMELEN);
		strncpy(name, DEFAULT_LOGDIR, MAX_LOG_NAMELEN);
		strncat(name, Name, MAX_LOG_NAMELEN-strlen(name)-1);

		int i;
		while ((i=strspn(name, FILENAME_GOODCHARS)) < strlen(name) ) {
			name[i] = FILENAME_BADCHARREPLACE;
		}
		fd = -1;
	}
	log(const log& copy) {
		*this = copy;	// FIXME: do not copy!
		eprintf("%s:%d: critical instance copy\n",__FILE__,__LINE__);
		//copy.fd = -1;	// FIXME: invalidate old LOG object (cause it not to close the file!)
	}

	~log() {
		if (fd>=0) {
			eprintf("closing logfile \'%s\'\n", name);
			if ( fsync(fd) < 0 )
				eperror("%s: ERROR: could not sync logfile on close", __FILE__);
			if ( close(fd) < 0 )
				eperror("%s: ERROR: could not close logfile properly", __FILE__);
		}
//		else {
//			eprintf("%s:%d: instance delete without file close!\n",__FILE__,__LINE__);
//		}
	}

	bool isOK() {
		return (fd>=0);
	}

	int openfile() {
		char namecopy[MAX_LOG_NAMELEN];
		strcpy(namecopy, name);
		char buffer[MAX_LOG_NAMELEN];
		char* filename = rindex(namecopy, '/')+1;
		char* dir;
		char* saveptr=NULL;
		
		bzero(buffer, MAX_LOG_NAMELEN);	// clear buffer;

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
		//eprintf("file %s\n", filename);
		strncat(buffer, filename, MAX_LOG_NAMELEN-strlen(buffer)-1);

		if ( strcmp(name, buffer) != 0) {
			eprintf("wanted name=\'%s\'\n", name);
			eprintf(" got buffer=\'%s\'\n", buffer);
			eprintf("ERROR: wrong logfile! (see %s:%d)\n", __FILE__,__LINE__);
			// this should actually never happen, but really bad things
			// will happen if the opened file and stored filename are out
			// of sync.
		}

		errno = 0;
		fd = open(name, O_WRONLY | O_CREAT | O_NOCTTY | O_APPEND, LOGFILE_PERMISSIONS);
		if (errno != 0) {
			eperror("ERROR: Could not open logfile \'%s\'", name);
			return fd;
		}
		eprintf("opened logfile \'%s\' as fd %d\n", name, fd);
		return fd;
	}

	log& operator= (const log& other) {
		strcpy(name, other.name);
		fd = other.fd;
		return *this;
	}

	bool operator== (const log& other) const {
		return (strcmp(name, other.name) == 0);
	}

	int print(const char* format, ...) {
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
		}
		va_end(va);
		return len;
	}
};

#endif
