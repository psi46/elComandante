/*
 * control parport data lines
 * Dennis Terhorst 06/2006
 * mod 01/2009 Dennis Terhorst
 */ 

#include <linux/ppdev.h>
#include <sys/ioctl.h>	// ioctl
#include <sys/types.h>	// open
#include <sys/stat.h>	// open
#include <fcntl.h>	// open
#include <unistd.h>	// close
#include <stdio.h>	// perror
#include <signal.h>	// sigaction

#include "lpt.h"
#include <errno.h>

int parfd;

void lpt_claimtimeout(int sig) {
	printf("ERROR: parport claim interrupted by timeout, aborting request.\n");
}

//
// initialize parport device
// returns: parfd or -1 fail (see errno)
// 
int lpt_open(const char* dev)
{
	//int parfd=-1;
	int i;

	struct sigaction act;				// install signal handler
	struct sigaction oldact;
	unsigned int oldalarm;
	act.sa_handler = lpt_claimtimeout;
	sigemptyset(&(act.sa_mask));
	act.sa_flags=0;
	int sighand_installed;
	if (! (sighand_installed = (sigaction(SIGALRM, &act, &oldact)==0)) ) {
		printf("warning: failed to setup signal handler!\n");
	} else {
		oldalarm = alarm(3);	// timeout in 3 seconds
	}

	printf("opening parport %s\n", dev);
	if ( (parfd = open(dev, O_RDWR)) < 0 ) {	// open
		perror("could not open parport");
		return parfd;
	}

	printf("claim parport...\n");			// claim
	if ( ioctl(parfd, PPCLAIM, &i) != 0 ) {
		perror("could not claim parport");
	}


	if ( sighand_installed ) {			// restore old handler, if installed
		sigaction(SIGALRM, &oldact, NULL);
		alarm(oldalarm);	// NOTE: upto 3 seconds lost here
	}

	if ( parfd >= 0 ) {				// info
		printf("successfully opened %s as fd%d\n", dev, parfd);
	}

	return parfd;
}

typedef enum {WRITE=0, READ=1} datadir_t;
int lpt_setDataDir(int parfd, datadir_t dir) {
	//int i;
	//dir = 0; // Tri-State '''aus''' (Der PC treibt die Data-Leitungen)
	//dir = 1; // Tri-State '''an'''  (Der PC treibt die Data-Leitungen '''nicht''')
	if ( parfd>0 ) {
		return ioctl( parfd, PPDATADIR, &dir );
	}
	errno = EINVAL;
	return -1;
}

//
// Write a bit pattern to the data lines
// returns: 0 on success.
//
int lpt_setdata(int parfd, char bitpat)
{
	if (parfd<0) {
		printf("WARNING: Setdata possible! file descriptor invalid.\n");
		errno = EINVAL;
		return -1;
	}
	printf("setting parport = 0x%02x (fd=%d)\n", bitpat, parfd);
	return ioctl(parfd, PPWDATA, &bitpat);
}


//
// Parport pin level functions
//
int lpt_getControl(int parfd, unsigned char *r) {
	if ( parfd>0 && r != NULL ) {
		return ioctl( parfd, PPRCONTROL, r ); // liest das "control" Register
	}
	errno = EINVAL;
	return -1;
}
int lpt_getStatus(int parfd, unsigned char *r) {
	if ( parfd>0 && r != NULL ) {
		return ioctl( parfd, PPRSTATUS , r ); // liest das "status"  Register
	}
	errno = EINVAL;
	return -1;
}
int lpt_getData(int parfd, unsigned char *r) {
	if ( parfd>0 && r != NULL ) {
		return ioctl( parfd, PPRDATA   , r ); // liest das "data"    Register
	}
	errno = EINVAL;
	return -1;
}

int lpt_setControl(int parfd, unsigned char r) {
	if ( parfd>0 ) {
		return ioctl( parfd, PPWCONTROL, &r ); // schreibt das "control" Register
	}
	errno = EINVAL;
	return -1;
}
int lpt_setData(int parfd, unsigned char r) {
	if ( parfd>0 ) {
		return ioctl( parfd, PPWDATA   , &r ); // schreibt das "data"    Register
	}
	errno = EINVAL;
	return -1;
}


//
// close parport device and zero all lines
// returns: 0, -1fail
//
int lpt_close() //int parfd)
{
	if (parfd <0) {
		errno = EINVAL;
		return -1;
	}
	int ret=0;
	lpt_setdata(parfd, 0);	// turn off all channels
	if ( ioctl( parfd, PPRELEASE, NULL ) < 0 ) {
		printf("WARNING: Release of parport failed!\n");
	}
	printf("closing fd%d\n",parfd);
	if ( (ret=close(parfd)) < 0 )
		perror("parport close failed:");

	return ret;
}
/*
	char *dev="/dev/parport0";
	
	
                // claim the port
                printf(" LPT: claim device %s...", dev);
                parfd = open(dev, O_RDWR );
                if (parfd < 0) {
                                printf("could not claim device");
                                exit(1);
                }
                // we need exclusive rights as we do not set the control lines //
                //ioctl(parfd, PPEXCL, &i)&&die("ERROR: request for exclusive rights failed\n");
        
	}        if ( ioctl(parfd, PPCLAIM, &i) != -1 )
                        printf("ok\n");

               lpt_setdata( bits->on );

*/
