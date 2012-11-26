
#include <stdio.h>
//#include <errno.h>
//#include <unistd.h>	// read

//#include <sys/socket.h>	// socket
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>	// inet_aton

#include "packet_t.h"
#include "sclient.h"
#include "error.h"

int main(int argc, char *argv[]) {

	sclient	me;

	#define BUFSIZE	256
	char buffer[BUFSIZE];
	int  buflen=0;

	// get name of packets
	if ( argc >= 2 ) {
		strncpy(buffer, argv[1], BUFSIZE);
		buflen=strlen(buffer);
		for (int i=0; i<buflen; i++) {
			if ( buffer[i] == 13 || buffer[i] == 10 ) {
				buffer[i]=0;
				break;
			}
		}
		buflen=strlen(buffer);
	} else {
		// return -1 here
		printf("send packets as: ");
		fgets(buffer, BUFSIZE, stdin);
		buflen=strlen(buffer);
		for (int i=0; i<buflen; i++) {
			if ( buffer[i] == 13 || buffer[i] == 10 ) {
				buffer[i]=0;
				break;
			}
		}
		buflen=strlen(buffer);
		printf("sending packets as: '%s'\n", buffer);
	}
	
	// init default packet
	packet_t packet;
	packet.setName(buffer);
	
	if(argc==3){
		strncpy(buffer, argv[2], BUFSIZE);
		buflen=strlen(buffer);
		for (int i=0; i<buflen; i++) {
			if ( buffer[i] == 13 || buffer[i] == 10 ) {
				buffer[i]=0;
				break;
			}
		}
		strcat(buffer, "\n");
		buflen=strlen(buffer);
		packet.setData(buffer, buflen);
		me.sendpacket(packet);
	}else{
		// main loop
		while ( true ) {
			buffer[0]=0;
			fgets(buffer, BUFSIZE, stdin);
			buflen = strlen(buffer);
			if ( buffer[0] == 'x' || feof(stdin)) break;

			packet.setData(buffer, buflen);

			me.sendpacket(packet);
		}
	}

	return 0;
}

