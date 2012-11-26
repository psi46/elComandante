/**
 *	selectable.cpp	09-Oct-2007
 *	Dennis Terhorst <dennis.terhorst@rwth-aachen.de>
 */


/* According to POSIX 1003.1-2001 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

//       int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

//       int pselect(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask);

//       FD_CLR(int fd, fd_set *set);
//       FD_ISSET(int fd, fd_set *set);
//       FD_SET(int fd, fd_set *set);
//       FD_ZERO(fd_set *set);

#include "selectable.h"
#include "error.h"
#include <errno.h>

#define DEBUG	0

// declaration of static members
int selectable::nosel=0;
selectable* selectable::sel[MAX_SELECTABLE];

selectable::selectable() {
		ready=0;
		thissel = nosel;
		sel[nosel++] = this;
		if (DEBUG) eprintf("number of selectables now %d\n", nosel);
	}

selectable::~selectable() {
		sel[thissel] = sel[nosel];
		sel[--nosel] = NULL;
//		nosel--;
//		eprintf("WARNING: ~selectable() does not work as expected (hole in array)!\n"); //FIXME
	}

/// this is the same as selectable::run() but with timeout used in select-statement
int selectable::run() { return selectable::run(-1,-1); }

/* FYI:       struct timeval {
                  long    tv_sec;          seconds 
                  long    tv_usec;         microseconds 
              };
*/

/**
 * \brief wait for next file descriptor to become ready
 *
 * This function does the actual select call. The fd_sets are build from all
 * instanciated selectables and timeout is set to the given values.
 * Each selectables ready is set and can be checked with isready().
 * \return number of ready fds (as returned by select), zero for timeout and -1 on error.
 * If no selectables are instanciated zero will be returned.
 */
int selectable::run(long sec, long usec) {
		if (nosel < 1) return -1;

		int ret = -1;
		fd_set readfds;		FD_ZERO(&readfds);
		fd_set writefds;	FD_ZERO(&writefds);
		fd_set exceptfds;	FD_ZERO(&exceptfds);

		// build fd_sets
		int maxfd = 0;
		for (int i=0; i<nosel; i++) { // for each selectable
			int fd, checks;
			fd = sel[i]->getfd();
			if (DEBUG) eprintf("sel[%d of %d] = fd%d for checks 0x%x\n", i, nosel, sel[i]->getfd(), sel[i]->getchecks());
			if (fd < 0) {
				if (DEBUG) eprintf("%s:%d WARNING: invalid file descriptor (%d)\n", __FILE__, __LINE__, fd);
				continue;
			}
			maxfd = ( fd>maxfd ? fd : maxfd );
			checks = sel[i]->getchecks();
			if ( checks & CHK_READ   ) FD_SET(fd, &readfds);
			if ( checks & CHK_WRITE  ) FD_SET(fd, &writefds);
			if ( checks & CHK_EXCEPT ) FD_SET(fd, &exceptfds);
			sel[i]->ready = 0;
		}
		// copy timeval
		
		struct timeval tout;
		struct timeval *tout_ptr = &tout;
		if ( sec < 0 ) {
			tout_ptr	= NULL;
		} else {
			tout.tv_sec	= sec;
			tout.tv_usec	= usec;
		}

		if (DEBUG) eprintf("maxfd=%d\n", maxfd);
		switch ( (ret=select(maxfd+1, &readfds, &writefds, &exceptfds, tout_ptr)) ) {
		case -1:	// ERROR
			if ( errno == EINTR ) return -1;
			eperror("select error");
			return -1;
		case 0:		// TIMEOUT
			return 0;
		default:	// FD READY
			//
			// call selectables which are ready
			//
			for (int i=0; i<nosel; i++) {	// check all fds
				int  fd = sel[i]->getfd();
				if ( fd < 0 ) continue;
				if (DEBUG) eprintf("checking selectable %d, fd%d\n",i, fd);
				if ( FD_ISSET(fd, &readfds) )	sel[i]->ready |= CHK_READ;
				if ( FD_ISSET(fd, &writefds) )	sel[i]->ready |= CHK_WRITE;
				if ( FD_ISSET(fd, &exceptfds) )	sel[i]->ready |= CHK_EXCEPT;
				if (DEBUG) eprintf("fd%d: now ready=0x%02X\n", fd, sel[i]->ready);
			}
			return ret;
		} 
	}

int selectable::isready(unsigned char type) const {
	return (ready & type);
}


