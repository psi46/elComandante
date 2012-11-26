/**
 *	clientinfo.cpp
 *	Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 *	Mon Feb 11 10:02:05 CET 2008
 */
#include <string.h>
#include <time.h>

#ifdef _WIN32
	#include <winsock2.h>
#else
	#include <sys/socket.h>	// socket
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>	// inet_aton
#endif

#include "error.h"
#include "clientinfo.h"
#include "abo.h"

clientinfo* clientinfo::list[MAX_CLIENTINFOS] = { NULL };
int clientinfo::noclientinfos = 0;

clientinfo::clientinfo(struct sockaddr_in *addr) {
	lastrecv = lastsend = 0;
	ppsrecv = ppssend = 0;
	strcpy(id, "unknown");

	memcpy(&address, addr, sizeof(address));

	myindex = noclientinfos;
	list[noclientinfos++] = this;		// add me to global list FIXME ovr
	if ( noclientinfos == MAX_CLIENTINFOS ) {
		eprintf("WARNING: reached MAX_CLIENTINFOS in %s:%d! Next instance will break array bounds!\n",
			__FILE__, __LINE__);
	}

	eprintf("new client: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
}

clientinfo::~clientinfo() {
	int i;
	while ( supply.length() > 0 ) {
		supply[0]->removeSupplier(this);
		supply -= supply[0];
	}
	while ( subscription.length() > 0 ) {
		subscription[0]->removeSubscriber(this);
		subscription -= subscription[0];
	}
	
	int me=-1;				// remove me from global list
	for (me=0; me<noclientinfos; me++) {
		if ( list[me] == this ) break;
	}
	if (me < 0) {
		eprintf("ERROR: %s:%d: Could not find me in list[]!\n",
			__FILE__, __LINE__);
		return;
	}
	noclientinfos--;
	list[me] = list[noclientinfos];
	list[me]->myindex = me;
	list[noclientinfos] = NULL;
}

int clientinfo::Supplies() {
	return supply.length();
}
int clientinfo::addSupply(abo* suppl) {
	return supply.add(suppl);
}
int clientinfo::removeSupply(abo* suppl) {
	return supply.remove(suppl);
}

int clientinfo::Subscriptions() {
	return subscription.length();
}
int clientinfo::addSubscription(abo* subscr) {
	return subscription.add(subscr);
}
int clientinfo::removeSubscription(abo* subscr) {
	return subscription.remove(subscr);
}

void clientinfo::received() { time(&lastrecv); return; }	// FIXME: calc pps here
void clientinfo::sent() { time(&lastsend); return; }		// FIXME: calc pps here

char* clientinfo::addrString() {
	static char buffer[128];
	sprintf(buffer, "%s:%d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	return buffer;
}

void clientinfo::setID(char* name) {
	strncpy(id, name, MAX_IDLEN);
	return;
}
const char* clientinfo::getID() {
	return (const char*)id;
}

int clientinfo::index() {
	return myindex;
/*	for (int i=0; i<noclientinfos; i++) {
		if (list[i] == this) return i;
	}
	eprintf("%s:%d: Internal error. Check list[] entries!\n");
	return -1;	// THIS MUST NEVER HAPPEN!
	*/
}

void clientinfo::printClients() {
	printf("%d client%s%s\n", noclientinfos, (noclientinfos==1?"":"s"), (noclientinfos>0?":":""));
	for (int i=0; i<noclientinfos; i++) {
		printf("  -> %s (last send: %ld seconds ago)\n", list[i]->addrString(), time(NULL)-list[i]->lastrecv);
		for (int a=0; a<list[i]->supply.length(); a++) {
			printf("     supplies   %s\n", list[i]->supply[a]->name);
		}
		for (int a=0; a<list[i]->subscription.length(); a++) {
			printf("     subscribed %s\n", list[i]->subscription[a]->name);
		}
	}
}
