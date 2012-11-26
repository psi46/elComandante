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

#define RWTH_WEATHER_ABO	"/rwth/weather"	///< abo where the weather should be posted
#define WEATHER_INTERVAL 	5*60		///< interval for submitting the weather in seconds
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
	time_t lastWeatherTime = 0;	///< last weather submit time
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
			if (time(NULL)>lastWeatherTime + WEATHER_INTERVAL ) {
				time(&lastWeatherTime);
				std::string data;
				curl_global_init(CURL_GLOBAL_WIN32);
				CURL *curl;
				curl = curl_easy_init();
				if(!curl) exit(1);
				curl_easy_setopt(curl, CURLOPT_URL, "http://www.klimageo.rwth-aachen.de/wtst/timecheck.php");
				curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writeCallback);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA    , &data);
				curl_easy_perform(curl); // Lass' krachen!

				int no_response = data.find("Gegenw&auml;rtig erfolgt keine Anzeige der aktuellen Wetterdaten");
				if(no_response > 0 && no_response < std::string::npos){
					eprintf("no data avaible!\n");
					return 0;
				}

				// HTML tags ersetzen
				size_t start = data.find("<");
				size_t end = data.find(">");
				int test;
				while(start!=std::string::npos && end!=std::string::npos){
					data.replace(start,end-start+1,"");
					start = data.find("<");
					end = data.find(">");
				}    
				// replace all html umlaute
				replace_all(data, "&uuml;", "ü");
				replace_all(data, "&auml;", "ä");
				replace_all(data, "&ouml;", "ö");
				replace_all(data, "&Uuml;", "Ü");
				replace_all(data, "&Auml;", "Ä");
				replace_all(data, "&Ouml;", "Ö");
				replace_all(data, "&nbsp;", " ");
				replace_all(data,"\r","");
				replace_all(data,"\n\n","");

				start = data.find("Lufttemperatur")+14+2;
				end = data.find("C(")-2;
				float temp = atof(data.substr(start, end-start).c_str());

				start = data.find("Relative Feuchte")+16+2;
				end = data.find("%(")-1;
				float humi = atof(data.substr(start, end-start).c_str());

				start = data.find("Windgeschwindigkeit")+19+2;
				end = data.find("m/s")-1;
				float wind_velocity = atof(data.substr(start, end-start).c_str());

				start = data.find("Windrichtung")+12+2;
				end = data.find("°");
				float wind_direction = atof(data.substr(start, end-start).c_str());

				start = data.find("Luftdruck")+9+2;
				end = data.find("hPaca")-1;
				float pressure = atof(data.substr(start, end-start).c_str());

				me.aprintf(RWTH_WEATHER_ABO,"%3.1fdegC %3.0f%% %3.1fm/s %3.1f° %4.1fhPa\n",
					temp,humi,wind_velocity,wind_direction,pressure);
				/*
				std::cout << "temperature: " << temp << " degC" << endl;
				std::cout << "humidity:    " << humi << " %" << endl;
				std::cout << "velocity:    " << wind_velocity << " m/s" << endl;
				std::cout << "direction:   " << wind_direction << "°" << endl;
				std::cout << "pressure:    " << pressure << " hPa" << endl;
				*/
				printf("weather sucessfully delivered\n");
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
