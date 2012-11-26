/**
 * \file rwth_weather.cpp
 * \brief bridge from the HTTP output of the RWTH weather station to the subserver
 * \author Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */

#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>	// read, STD*_FILENO
#include <stdlib.h>	// atexit
#include <signal.h>	// signal()
#include <sys/select.h>	// select()
#include <string>

#include "../../../sclient.h"
#include "../../../packet_t.h"
#include "../../../error.h"
#include "../../../signalnames.h"

#include <curl/curl.h>

#include "../replace_all.h"

/**
 * \brief write callback for CURL
 */
int curl_writeCallback(char *inBuffer, size_t size, size_t count, std::string *outBuffer){
	outBuffer->append(inBuffer);
	return count*size;
}
void cleanexit() {
	eprintf("clean exit\n");
}

void sigint_handler(int sig) {
	eprintf("received %s(%d)\n", SIGNAME[sig], sig);
}

#define TEK_DATA_ABO	"/tek/data"	///< abo where the weather should be posted
#define DATA_INTERVAL	2*60		///< break between requesting to data sets
#define MAX_VALUES	20000		///< Max Values returned from TEK Scope

int main(int argc, char *argv[]){
	atexit(cleanexit);
	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);
	packet_t packet;
	sclient me;

	if (!me.isOK()) {
		eprintf("could not initialize sclient!\n");
		return -1;
	}

	me.setid(argv[0]);

	#define BUFSIZE	256
	char buffer[BUFSIZE];
	int  buflen=0;

	fd_set* readfds = new fd_set();
	fd_set* writefds= new fd_set();
	fd_set* errorfds= new fd_set();
	fd_set* allfds  = new fd_set();

	int maxfd;

	FD_ZERO(allfds);
	FD_SET(me.getfd(), allfds);
	FD_SET(STDIN_FILENO, allfds);
	maxfd=STDIN_FILENO;
	if (me.getfd() > maxfd) maxfd=me.getfd();

	struct timeval timeout;
	int noready;
	time_t lastDataTime = 0;	///< last weather submit time
	while( true ){

		timeout.tv_sec=5;
		timeout.tv_usec=0;

		*readfds = *allfds;
		noready = select(maxfd+2, readfds, writefds, errorfds, &timeout);

		if ( noready < 0 ) {	///// ERROR
			eprintf("select returned %d, error %d: %s \n", noready, errno, strerror(errno));
			break;
		} else if ( noready == 0 ) {	///// TIMEOUT
			// boring....
			if (time(NULL)>lastDataTime + DATA_INTERVAL ) {
				std::string data;
				curl_global_init(CURL_GLOBAL_WIN32);
				CURL *curl;
				curl = curl_easy_init();
				if(!curl) exit(1);

                                curl_easy_setopt(curl, CURLOPT_URL, "http://tek3b02/getwfm.isf");
                                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "command=select:ch1%20on&"
					"command=save:waveform:fileformat%20spreadsheet&"
					"wfmsend=Get");

                                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeCallback);
                                curl_easy_setopt(curl, CURLOPT_WRITEDATA    , &data);
                                curl_easy_perform(curl);

//				std::cout << "CURL:" << std::endl;
//	                        std::cout << data << std::endl;

				size_t start = 0;
				size_t end   = data.find("\n");
				std::string out;

				double v_time[MAX_VALUES]={0};
				double value[MAX_VALUES]={0};
				
				int cnt = 0;
				while(cnt < MAX_VALUES && (start!=std::string::npos && end!=std::string::npos) ){
					out = data.substr(start,end-start); 
					sscanf(out.c_str(),"%lf,%lf", &v_time[cnt], &value[cnt]);
//					me.aprintf(TEK_DATA_ABO, "%d %lf\t%lf\n", cnt, v_time[cnt]*1e7, value[cnt]);
					start=end+1;
					end = data.find("\n", start);
					cnt ++;
				}

				cnt--; // because we have one to much
				me.aprintf(TEK_DATA_ABO, "%ld read %d data\n", time(NULL), cnt);

				curl_easy_cleanup(curl);
	
				time(&lastDataTime);
			}
		} else {// if noready > 0)	///// DATA
			if ( FD_ISSET(STDIN_FILENO, readfds) ) {
				buffer[0]=0;
				fgets(buffer, BUFSIZE, stdin);
				buflen = strlen(buffer);
				if ( buffer[0] == 'x' || feof(stdin)) break;
			}
			if ( FD_ISSET(me.getfd(), readfds) ) {
				packet_t rxpacket;
				// read packet
				if ( me.recvpacket(rxpacket) < 0 ) {
					eprintf("Error receiving a packet\n");
					continue;
				}
				//printf("%1$.*2$s", rxpacket.data(), rxpacket.datalen());
			}
		}
	}
	eprintf("client exiting.\n");
	return 0;
} 
