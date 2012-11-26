/**
 * sclient.cpp
 * Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * 18.Feb.2008
 */

#ifdef _WIN32	// WINDOWS
	#include <winsock.h>
	#include <winsock2.h>
	#include <io.h>
#else			// LINUX
	#include <sys/socket.h>	// socket
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>	// inet_aton
#endif

#include <unistd.h>	// close()
#include <stdlib.h>	// atoi()

#include "sclient.h"
#include "error.h"

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


#define ENVBUFLEN	512
sclient::sclient() {
	//const char* serverip=SUBSERVER_DEFAULT_ADDR, const int serverport=SUBSERVER_DEFAULT_PORT) {
	char buffer[ENVBUFLEN];
	char *svr = getenv(SUBSERVER_ENVNAME);
	char *port_ptr;
	int port;
	int envfound=0;
	if (svr != NULL) {	// server from environment
		strncpy(buffer, svr, ENVBUFLEN-1);
		buffer[ENVBUFLEN-1] = 0;
		if ( (port_ptr = strchr(svr,':') ) != NULL) {
			*port_ptr=0;
			port_ptr++;
			port = atoi(port_ptr);
			if (port<65536 && port > 0) {
				init(svr, port);
				envfound=1;
			}
		}
	}
	if (! envfound) {	// use default compiled in server
		init(SUBSERVER_DEFAULT_ADDR,SUBSERVER_DEFAULT_PORT);
	}
}

sclient::sclient(const char* serverip, const short serverport) {
	init(serverip, serverport);
}
void sclient::init(const char* serverip, const short serverport) {
#ifdef _WIN32
    /* Initialisiere TCP für Windows ("winsock") */
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 0);
    if (WSAStartup(wVersionRequested, &wsaData) != 0){
	  eprintf("Error while init winsock\n");
      exit(1);
    }
	// Check version
	if (wsaData.wVersion != wVersionRequested)
	{
		eprintf("\nWinSock version not supported\n");
		return;
	}
#endif
	//eprintf("opening socket...\n");
	sfd = socket(PF_INET, SOCK_DGRAM, 0);
#ifdef _WIN32
	if ( sfd == SOCKET_ERROR) { eperror("ERROR: socket() call returned SOCKET_ERROR"); }
#else	
	if ( sfd < 0) { eperror("ERROR: socket() call returned %d", sfd); }
#endif

	//eprintf("  creating struct sockaddr_in svraddr...\n");
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(serverport);
#ifdef _WIN32
    unsigned long addr;
    if ((addr = inet_addr(serverip)) != INADDR_NONE) {
        /* serverip ist eine numerische IP-Adresse */
        memcpy( (char *)&svraddr.sin_addr, &addr, sizeof(addr));
    }
#else
	inet_aton(serverip, &(svraddr.sin_addr));
#endif

	//eprintf("  connect socket...\n");
	int connerr = connect(sfd, (struct sockaddr*)&svraddr, sizeof(svraddr));
	if (connerr != 0) { eperror("ERROR: connect returned %d:",connerr); }
	//fprintf(stderr,"connected to %s:%d\n", inet_ntoa(svraddr.sin_addr), ntohs(svraddr.sin_port));

	DefaultSendname[0]=0;
	
	for ( int i=0; i<MAX_CALLBACKS; i++ ) {	// clear callback registrations;
		rxcallback[i]=NULL;
	}
}

sclient::~sclient() {
	terminate();
	#ifdef _WIN32
    		closesocket(sfd);
    		/* Cleanup Winsock */
    		WSACleanup();
	#else
   		close(sfd);
	#endif
}

#ifdef _WIN32
SOCKET sclient::getfd() {
	return sfd;
}
#else
int sclient::getfd() const {
	return sfd;
}
#endif

bool sclient::isOK() {
	return (sfd>=0);
}

int sclient::sendpacket(packet_t& packet) {
	int ret;
	// packet is always sent in network byteorder
	packet.type = htons(packet.type);
	packet.namelength = htons(packet.namelength);
	if ((ret=send(sfd, (const char*)packet.raw, packet.length, 0)) < 0) {
		eperror("ERROR: local send() error");
	}
	// packet convert bytes back to keep packet constant
	packet.type = ntohs(packet.type);
	packet.namelength = ntohs(packet.namelength);
	return ret;
}

int sclient::recvpacket(packet_t& packet) {
	packet.addrlen=sizeof(packet.address);
	errno=0;
	packet.length = recvfrom(sfd, (char*)packet.raw, packet_t::MAX_LENGTH, 0, (struct sockaddr*)&(packet.address), &(packet.addrlen));
	if (errno != 0 || packet.length < 0 ) {
		eperror("recvfrom() returned %d", packet.length);
		packet.length = -1;
	}
	// packet is always stored locally in host byteorder
	packet.type = ntohs(packet.type);
	packet.namelength = ntohs(packet.namelength);
	return packet.length;
}

void sclient::setid(const char* name) {
	packet_t packet;
	char buffer[MAX_PACKETLEN];
	int blen = snprintf(buffer, MAX_PACKETLEN, "setid %s", name);
	
	packet.setName("");
	packet.setData(buffer, blen+1);	// +1 for terminating \0 of string.
	packet.type = PKT_MANAGEMENT;
	sendpacket(packet);

	return;
}

int sclient::subscribe(const char* name) {
	packet_t packet;
	packet.setName(name);
	packet.setData("",0);
	packet.type = PKT_SUBSCRIBE;
	return sendpacket(packet);
}
int sclient::unsubscribe(const char* name) {
	packet_t packet;
	packet.setName(name);
	packet.setData("",0);
	packet.type = PKT_UNSUBSCRIBE;
	return sendpacket(packet);
}

int sclient::supply(const char* name) {
	packet_t packet;
	packet.setName(name);
	packet.setData("",0);
	packet.type = PKT_SUPPLY;
	return sendpacket(packet);
}
int sclient::unsupply(const char* name) {
	packet_t packet;
	packet.setName(name);
	packet.setData("",0);
	packet.type = PKT_UNSUPPLY;
	return sendpacket(packet);
}

int sclient::setDefaultSendname(const char* name) {
	strncpy(DefaultSendname, name, MAX_PACKETLEN);
	//eprintf("DefaultSendname set to \"%s\"\n", DefaultSendname);
	return strlen(DefaultSendname);
}

int sclient::aprintf(const char* name, const char* fmt, ...) {
	packet_t tx;
	char data[MAX_PACKETLEN];
	int dlen;

	va_list va;
	va_start(va, fmt);
	dlen = vsnprintf(data, MAX_PACKETLEN, fmt, va);
	va_end(va);

	tx.setName(name);
	tx.setData(data, dlen);
	tx.type = PKT_DATA;

	return sendpacket(tx);
}

// set() is a experimental function
int sclient::set(const char* name, char* key, double value) {
	packet_t tx;
	char data[MAX_PACKETLEN];
	int dlen;

	dlen = snprintf(data, MAX_PACKETLEN, "set %s\t%lf\n", key, value);

	tx.setName(name);
	tx.setData(data, dlen);
	tx.type = PKT_SETDATA;

	return sendpacket(tx);
}

int sclient::printf(const char* fmt, ...) {
	packet_t tx;
	char data[MAX_PACKETLEN];
	int dlen;

	va_list va;
	va_start(va, fmt);
	dlen = vsnprintf(data, MAX_PACKETLEN, fmt, va);
	va_end(va);

	if ( DefaultSendname[0] != 0 ) {
		tx.setName(DefaultSendname);
		tx.setData(data, dlen);
		tx.type = PKT_DATA;

		return sendpacket(tx);
	}

	eprintf("WARNING: DefaultSendname not set and sclient::printf() used!\n");
	eprintf("printf\t%s\n", data);
	return 0;
}

int sclient::register_rxcall(void (*func)(packet_t), const char* aboname) {
	int i=-1;
	if ( func == NULL ) {		// REMOVE
		while ( rxcallback[++i] != NULL && i<MAX_CALLBACKS ) {
			if ( strcmp(aboname, rxcallback[i]->abo) == 0 ) {
				delete rxcallback[i];
				while ( (rxcallback[i++]=rxcallback[i+1]) != NULL && i<MAX_CALLBACKS-1 );
			}
		}
	} else {			// ADD
		i=0;
		while ( rxcallback[i] != NULL && i<MAX_CALLBACKS ) i++;
		if ( i==MAX_CALLBACKS ) return 1;
		//eprintf("registering rxcallback[%d] for %s\n", i, aboname);
		rxcallback[i] = new rxcall_t<void>(aboname, func);
		subscribe(aboname);
	}
	return 0;
}

void sclient::do_rxcalls(packet_t packet) {
	int i=-1;
	//eprintf("do_rxcalls()\n");
	while ( rxcallback[++i] != NULL && i<MAX_CALLBACKS ) {
		//eprintf("    callback[%d]\n", i);
		if ( strcmp(packet.name(), rxcallback[i]->abo) == 0 ) {
			//eprintf("    run()\n");
			rxcallback[i]->run(packet);
		}
	}
}

void sclient::terminate() {
	packet_t packet;
	packet.setData("",0),
	packet.type = PKT_CLIENTTERM;
	sendpacket(packet);
}

