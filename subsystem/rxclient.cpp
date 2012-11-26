
#include <stdio.h>
#include <errno.h>
#include <unistd.h>	// read
#include <stdlib.h>	// atexit
#include <signal.h>	// signal()

#include "sclient.h"
#include "packet_t.h"
#include "error.h"

// set local address to one of INADDR_ANY, INADDR_LOOPBACK, inet_aton(), inet_addr(), gethostbyname()
#define SVR_ADDR		"127.0.0.1"
#define SVR_PORT		12334

int wantexit = 0;

void cleanexit() {
	printf("clean exit\n");
	wantexit = 1;
}

void sigint_handler(int sig) {
	printf("received SIGINT\n");
	wantexit = 1;	// will need one more packet to take effect
}

int main(int argc, char *argv[]) {
	fprintf(stderr, "client starting...\n");

	atexit(cleanexit);
	signal(SIGINT, sigint_handler);
	
	sclient me(SVR_ADDR, SVR_PORT);
	packet_t packet;


	#define BUFSIZE	256
	char buffer[BUFSIZE];
	int  buflen=0;

	// get name of packets
	if ( argc == 2 ) {
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
		fprintf(stderr, "subscribe packets named: ");
		fgets(buffer, BUFSIZE, stdin);
		buflen=strlen(buffer);
		for (int i=0; i<buflen; i++) {
			if ( buffer[i] == 13 || buffer[i] == 10 ) {
				buffer[i]=0;
				break;
			}
		}
		buflen=strlen(buffer);
	}
	fprintf(stderr, "subscribing '%s'\n", buffer);


	// subscribe packet
	packet.setName(buffer);
	packet.setData("", 0);
	packet.type = PKT_SUBSCRIBE;
	me.sendpacket(packet);

	// main loop
	while (wantexit == 0) {

		// read packet
		printf("waiting for packet\n");
		if ( me.recvpacket(packet) < 0 ) {
			printf("got error\n");
			continue;
		}
		//packet.print();
		printf("%1$.*2$s", packet.data(), packet.datalen());
	}

	// unsubscribe
	packet.setData("",0),
	packet.type = PKT_UNSUBSCRIBE;
	me.sendpacket(packet);

	
	printf("client exiting.\n");
	return 0;
}

