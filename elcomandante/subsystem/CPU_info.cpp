#include "CPU_info.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <stdlib.h>
#include <string.h>


float CPU_info::load = 0;
long CPU_info::user=0, CPU_info::system=0, CPU_info::nice=0, CPU_info::idle=0;

FILE *CPU_info::fp_stat = NULL;
FILE *CPU_info::fp_loadavg = NULL;

CPU_info::CPU_info(){
#ifndef MAC_OS
	fp_stat= fopen ("/proc/stat", "r");
	fp_loadavg= fopen ("/proc/loadavg", "r");
	
	if ( (!fp_stat) || (!fp_loadavg) ) {
		fprintf (stderr, "Error while opening the CPU_info files\n");
		exit(1);
	}
#endif
#ifdef MAC_OS

#endif
}

CPU_info::~CPU_info(){
#ifndef MAC_OS
	fclose (fp_stat);
	fclose (fp_loadavg);
#endif
}
/**
 * Getting the CPU_info
 */
int  CPU_info::getStats() {

#ifndef MAC_OS
	// Reset FilePointer
	rewind (fp_stat);
	rewind (fp_loadavg);

	char temp[128];
	while (fgets (temp, 128, fp_stat)) {
		if (strstr (temp, "cpu")) {
			sscanf (temp, "cpu %ld %ld %ld %ld", &user, &system, &nice, &idle );
			break;
		}
	}
	fscanf (fp_loadavg , "%f", &load);

/* 	ethernet 
	#define PROC_PATH	"/proc/net/dev"
	#define MAXLEN		255 		
	#define INTERFACE	"eth0"
	if ((fp=fopen(PROC_PATH, "r"))==NULL) return (-1); 
		// header 
		fgets (buffer, MAXLEN, fp);
		fgets (buffer, MAXLEN, fp);
		while (!feof(fp)) {
			fgets (buffer, MAXLEN, fp);
			if (strstr (buffer, interface)) {
				fclose (fp);
				fprintf (stdout,buffer);
				break;
			}
		}
	}*/
	return 0;
#endif
}
float CPU_info::actual()  { return user + system + nice;  }
float CPU_info::total()   { return user + system + nice + idle;   }
float CPU_info::average() { return load; }

char* CPU_info::getIP(){
	char hostname[MAXHOSTNAMELEN];
	gethostname(hostname, MAXHOSTNAMELEN);
	hostent * record = gethostbyname(hostname);
	if (record==NULL){
		herror("gethostbyname failed");
		exit(1); 
	}
	in_addr * address=(in_addr * )record->h_addr;
	return inet_ntoa(* address);


}

char* CPU_info::getHostname(){
	char* hostname;
	gethostname(hostname, MAXHOSTNAMELEN);
	return hostname;
}

