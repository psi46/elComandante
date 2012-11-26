#include <stdio.h>	// printf
#include <unistd.h>	// usleep
#include "lpt.h"
#include <daemon.h>
#include <time.h>	// gettimeofday, timersub
#include <sys/time.h>	// gettimeofday, timersub

volatile int wantexit;

void sighand(int sig) {
	printf("catched signal %d, exiting...", sig);
	wantexit++;
}

int main(int argc, char* argv[]) {
	int fd=0;
	printf("Hallo Welt\n");
	wantexit=0;
	
	catch_signals(sighand);

	struct timeval start;
	struct timeval stop;
	gettimeofday(&start, NULL);

	if (argc < 2) {
		printf("give parport device as parameter!\n");
		return -1;
	}

	if ( (fd=lpt_open(argv[1])) < 0) {
		perror("could not open parallel port");
		return -1;
	}

	gettimeofday(&stop, NULL);
	timersub(&stop, &start, &stop);
	printf("stat: lpt_open took %d.%06d seconds.\n", stop.tv_sec, stop.tv_usec);

	int r=0;
	int i=0;
	int delay=50; // usec
	unsigned short brightness[] = { 0x0000, 0x0101, 0x1111, 0x2929, 0x5555, 0x6b6b, 0x7777, 0x7F7F, 0xFFFF};
	unsigned short value[] = { 1,1,2,2,3,4,6,7,8,7,6,4,3,2,1,1};
	int nov = 16;
	unsigned char data;
	int val;
	for (val=000; val < 1000 && wantexit==0; ++val) {
		printf("val %d \n", val);
//		gettimeofday(&start, NULL);	// start timer
		for (i=0; i<32 && wantexit==0; i++) {
			data = 0;
			data |= (brightness[value[(val+0)%nov]]>>(i%8)) & 1 ;
			data |= (brightness[value[(val+1)%nov]]>>(i%8)) & 2 ;
			data |= (brightness[value[(val+2)%nov]]>>(i%8)) & 4 ;
			data |= (brightness[value[(val+3)%nov]]>>(i%8)) & 8 ;
			data |= (brightness[value[(val+4)%nov]]>>(i%8)) & 16 ;
			data |= (brightness[value[(val+5)%nov]]>>(i%8)) & 32 ;
			data |= (brightness[value[(val+6)%nov]]>>(i%8)) & 64 ;
			data |= (brightness[value[(val+7)%nov]]>>(i%8)) & 128;
			r = lpt_setData(fd, data);
			/*if (r<0) {
				perror("     returned <0");
			} else {
				printf("     return %d\n", r);
			}*/
			usleep(1);
		}
/*		gettimeofday(&stop, NULL);	// stop timer
		timersub(&stop, &start, &stop);
		printf("stat: lpt_setData took       %d.%06d seconds.\n", stop.tv_sec, stop.tv_usec);
		start.tv_sec=0;
		start.tv_usec=256*delay;
		printf("stat: write delays took      %d.%06d seconds.\n", start.tv_sec, start.tv_usec);
		timersub(&stop, &start, &stop);
		printf("stat: lpt_setData real time  %d.%06d seconds.\n", stop.tv_sec, stop.tv_usec);
*/
	}
	gettimeofday(&start, NULL);	// start timer
	if ( lpt_close(fd) < 0) {
		perror("could not close parallel port");
		return -1;
	}
	gettimeofday(&stop, NULL);	// stop timer
	timersub(&stop, &start, &stop);
	printf("stat: lpt_close took %d.%06d seconds.\n", stop.tv_sec, stop.tv_usec);
	return 0;
}
