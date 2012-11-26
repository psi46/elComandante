/**
 * \file speiseplan.cpp
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

#include <subsystem/sclient.h>
#include <subsystem/packet_t.h>
#include <subsystem/error.h>
#include <subsystem/signalnames.h>

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

#define RWTH_FOOD_ABO		"/rwth/food"	///< abo where the food should be posted
#define FOOD_INTERVAL 		5*60		///< interval for submitting the food in seconds
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
	time_t lastfoodTime = 0;	///< last food submit time
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
			if (time(NULL)>lastfoodTime + FOOD_INTERVAL ) {
				time(&lastfoodTime);

				std::string data;
				curl_global_init(CURL_GLOBAL_WIN32);
				CURL *curl;
				curl = curl_easy_init();
				if(!curl) exit(1);

				curl_easy_setopt(curl, CURLOPT_URL, "http://www.studentenwerk-aachen.de/essen/"
					"speiseplan.asp");
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "elec_Wann=Heute&Abschicken=Speisepl%E4ne"	
					"+anzeigen%21&allec=&newcoys=&newpdfs=MensaVita%3A%3A&elec_MensaVita=1");

				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA    , &data);
				curl_easy_perform(curl); 

//				std::cout << data << std::endl;

				// HTML tags ersetzen
				size_t start = data.find("<");
				size_t end = data.find(">");
/*				int test;
				while(start!=std::string::npos && end!=std::string::npos){
					data.replace(start,end-start+1,"");
					start = data.find("<");
					end = data.find(">");
				}
*/
				start = data.find("Uhr)")+4; 
				end   = data.find("</table>");
				std::string out = data.substr(start,end-start); 

				replace_all(out, "<br>", "\n");
				replace_all(out, "&nbsp;", " ");
				replace_all(out, "<br>", "\n");
				replace_all(out, "  ", "");
				replace_all(out, "\t", "");

				replace_all(out, "<br/>", "");
				replace_all(out, "</td>", "");
				replace_all(out, "</tr>", "");
				replace_all(out, "</nobr>","\n");
				replace_all(out, "<font color=#000000 size=1>", "");
				replace_all(out, "<td valign=\"top\">", "");
				replace_all(out, "<font size=\"1\">", "");
				replace_all(out, "<b>", "");
				replace_all(out, "</b>", "");
				replace_all(out, "</font>", "");
				replace_all(out, "<nobr>", "");
				replace_all(out, "&euro;", "Euro");

  
				// replace all html umlaute
				replace_all(out, "&uuml;", "ü");
				replace_all(out, "&auml;", "ä");
				replace_all(out, "&ouml;", "ö");
				replace_all(out, "&Uuml;", "Ü");
				replace_all(out, "&Auml;", "Ä");
				replace_all(out, "&Ouml;", "Ö");
				replace_all(out, "&nbsp;", " ");
				replace_all(out,"\r","");
				replace_all(out,"\n\n","");

				//std::cout << out << std::endl;			
				me.aprintf(RWTH_FOOD_ABO,"%s\n",out.c_str());

				printf("food sucessfully delivered\n");
				curl_easy_cleanup(curl);

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
