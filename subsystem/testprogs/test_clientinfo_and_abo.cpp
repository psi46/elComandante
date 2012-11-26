
#include <stdio.h>
#include <errno.h>
#include <unistd.h>	// read

#include <sys/socket.h>	// socket
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>	// inet_aton

#include <string.h>

#include "error.h"


// set local address to one of INADDR_ANY, INADDR_LOOPBACK, inet_aton(), inet_addr(), gethostbyname()
#define MY_ADDR		"0.0.0.0"

#define MY_PORT		12334

using namespace std;

/*
struct sockaddr_in {
	sa_family_t    sin_family; // address family: AF_INET
	u_int16_t      sin_port;   // port in network byte order
	struct in_addr  sin_addr;  // internet address
};

// Internet address.
struct in_addr {
	u_int32_t      s_addr;     // address in network byte order
};

*/

// getsockopt(IP_MTU) // see man ip(7)


// get size of next message in bytes or zero if no dgm pending
//int value;
//if ( (error=ioctl(sfd, SIOCINQ, &value)) != 0 ) {
//	eperror("ERROR: ioctl(SIOCINQ) failed");
//	return 0;
//}

// get size of local send queue in bytes
//int value;
//if ( (error=ioctl(sfd, SIOCOUTQ, &value)) != 0 ) {
//	eperror("ERROR: ioctl(SIOCOUTQ) failed");
//	return 0;
//}

/*
#define MAX_INFOLEN	512
class infopacket {
public:
	int	type;
	int	len;
	char	data[MAX_INFOLEN];
};
*/

#include "clientinfo.h"
#include "abo.h"

int main() {

	struct sockaddr_in testaddr;
	testaddr.sin_family = AF_INET;
	testaddr.sin_port = htons(1032);
	inet_aton("192.168.100.101", &(testaddr.sin_addr));

	char name[] = "/lx3btpc02/camac/crate1/slot8/adcdata";
	int namelen = strlen(name);

	printf("*** create client\n");
	clientinfo* C = clientinfo::Client(&testaddr);
	clientinfo::printClients(); abo::printAbos();

	printf("*** create abo\n");
	abo* A = abo::Abo(name, namelen);
	clientinfo::printClients(); abo::printAbos();
	
	printf("*** client addSupply\n");
	C->addSupply(A);
	clientinfo::printClients(); abo::printAbos();

	printf("*** client supplies (2)\n");
	C->addSupply(A);
	clientinfo::printClients(); abo::printAbos();

	printf("*** create client 2\n");
	inet_aton("192.168.100.102", &(testaddr.sin_addr));
	clientinfo* C2 = clientinfo::Client(&testaddr);
	clientinfo::printClients(); abo::printAbos();

	printf("*** abo addSubscriber client2\n");
	A->addSubscriber(C2);
	clientinfo::printClients(); abo::printAbos();

	printf("*** delete client\n");
	delete C;
	clientinfo::printClients(); abo::printAbos();
	
	printf("*** create client\n");
	inet_aton("192.168.100.103", &(testaddr.sin_addr));
	C = clientinfo::Client(&testaddr);
	clientinfo::printClients(); abo::printAbos();

	printf("*** client addSupply() A\n");
	C->addSupply(A);
	clientinfo::printClients(); abo::printAbos();

	printf("*** delete A\n");
	delete A;
	clientinfo::printClients(); abo::printAbos();


	return 32;

	printf("Server starting...\n");

	printf("  creating socket...\n");
	int sfd = socket(PF_INET, SOCK_DGRAM, 0);
	if ( sfd < 0) { eperror("ERROR: socket() call returned %d", sfd); return -1; }

	printf("  building struct sockaddr_in...\n");
	struct sockaddr_in myaddr, src_addr;
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(MY_PORT);
	//inet_aton(SVR_ADDR, &(myaddr.sin_addr));
	myaddr.sin_addr.s_addr = INADDR_ANY;

	int binderr = bind(sfd, (struct sockaddr*)&myaddr, sizeof(myaddr));
	if (binderr != 0) { eperror("ERROR: bind() returned %d:",binderr); return -1; }
	printf("  successfully bound to address %s:%d\n", inet_ntoa(myaddr.sin_addr), ntohs(myaddr.sin_port));


//	int listerr = listen(sfd, 5);
//	if (listerr != 0) { eperror("ERROR: listen() returned %d:",listerr); return -1; }
//	printf("listening for packets...\n");

	#define BUFSIZE	2048
	char buffer[BUFSIZE];
	int len =2314;
	socklen_t src_addr_len;

	while (buffer[0] != 'X') {

		//len = read(sfd, buffer, BUFSIZE);
		src_addr_len=sizeof(src_addr);
		len = recvfrom(sfd, buffer, BUFSIZE, 0, (struct sockaddr*)&src_addr, &src_addr_len);
		buffer[len] = 0;
		printf(">>> read len=%d\n", len);
		if ( len > 0 ) {
			buffer[BUFSIZE-1] = 0;
			for (int i = 0; i<len; i++) {
				if ( buffer[i]==10 ) buffer[i]='\\';
				if ( buffer[i]==13 ) buffer[i]='\\';
			}
			printf("%s:%d: %d bytes: '%s'\n", inet_ntoa(src_addr.sin_addr), ntohs(src_addr.sin_port), len, buffer);
		} else {
			eperror("WARNING: read returned <0");
		}
	}

	printf("Server exiting.\n");
	close(sfd);
	return 0;
}

