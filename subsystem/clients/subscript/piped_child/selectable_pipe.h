#ifndef SELECTABLE_PIPE_H
#define SELECTABLE_PIPE_H

#include "selectable.h"

#include <unistd.h>
#include <stdio.h>

typedef enum { READ=0, WRITE=1 } direction_t;

class pipe_t {
	int fd[2];
public:
	pipe_t() {
		fd[READ] = fd[WRITE] = -1;
	}

	int reopen() {
		this->close_all();
		//close_read();
		//close_write();
		return pipe(fd);
	}
	virtual ~pipe_t() {
		this->close_all();
	}
	int close_read()  { close(fd[READ]);  fd[READ]=-1;  return 0; }
	int close_write() { close(fd[WRITE]); fd[WRITE]=-1; return 0; }
	int close_all() {
		int ret=0;
		if (fd[READ] >=0)  ret+=close_read();
		if (fd[WRITE] >=0) ret+=close_write();
		return ret;
	}
	int rfd() const { return fd[READ]; }
	int wfd() const { return fd[WRITE]; }
};

template <direction_t DIRECTION>
class pipe_selectable : public pipe_t, public selectable {
public:
	pipe_selectable() : pipe_t(), selectable() {};
	virtual ~pipe_selectable() {};

	// selectable virtuals
	virtual int getfd() const {
		if ( DIRECTION==READ )  return this->rfd();
		if ( DIRECTION==WRITE ) return this->wfd();
		return -1;
	}
	virtual int getchecks() const {
		if ( DIRECTION==READ  && rfd()>0 )  return CHK_READ;
		if ( DIRECTION==WRITE && wfd()>0 )  return 0; // we do not want to check for CHK_WRITE, because this undermines select timeout;
		return -1;
	}
};

#endif //ndef SELECTABLE_PIPE_H
