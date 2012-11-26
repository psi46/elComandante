/**
 *	abo.cpp
 *	Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 *	Mon Feb 11 10:02:05 CET 2008
 */
#include <string.h>
#include <time.h>

#include "error.h"
#include "abo.h"
#include "clientinfo.h"
#include "packet_t.h"

abo* abo::list[MAX_ABOS] = { NULL };
int abo::noabos = 0;

abo::abo(char* aboname, int abonamelen) {
	mylog = NULL;	// no default logging
	loglevel=0;
	flags=0;
	lastrecv=0;

	name[0]=0;
	namelen =  (abonamelen<MAX_NAMELEN-1?abonamelen:MAX_NAMELEN-1);	// leave space for \0
	strncpy(name, aboname, namelen);
	name[namelen] = 0;

	myindex = noabos;
	list[noabos++] = this;		// add me to global list FIXME ovr
	if ( noabos == MAX_ABOS ) {
		eprintf("WARNING: reached MAX_ABOS in %s:%d! Next instance will break array bounds!\n",__FILE__, __LINE__);
	}
	eprintf("new abo: %s\n", name);
}

abo::~abo() {
	int i;
	while ( supplier.length() > 0) { //	for (i=0; i<nosuppliers;i++) {
		supplier[0]->removeSupply(this);
		supplier -= supplier[0];
		//removeSupplier(supplier[0]); //->removeSupply(this);
	}
	while ( subscriber.length() > 0) { //for (i=0; i<nosubscribers; i++) {
		subscriber[0]->removeSubscription(this);
		subscriber -= subscriber[0];
		//removeSubscriber(subscriber[0]); //->removeSubscription(this);
	}

	int me=-1;			// remove me from global list
	for (me=0; me<noabos; me++) {
		if ( list[me] == this ) break;
	}
	if (me < 0) {
		eprintf("ERROR: %s:%d: Could not find me in list[]!\n",
			__FILE__, __LINE__);
		return;
	}
	noabos--;
	list[me] = list[noabos];
	list[me]->myindex = me;
	list[noabos] = NULL;
}

int abo::Suppliers() { return supplier.length(); }

int abo::addSupplier(clientinfo* suppl) {
	return supplier.add(suppl);
}

int abo::removeSupplier(clientinfo* suppl) {
	return supplier.remove(suppl);
}

int abo::Subscribers() { return subscriber.length(); }

int abo::addSubscriber(clientinfo* subscr) {
	return subscriber.add(subscr);
}

int abo::removeSubscriber(clientinfo* subscr) {
	return subscriber.remove(subscr);
}

void abo::received() { lastrecv=time(NULL); return; }

int abo::getLogLevel() {
	return loglevel;
}
int abo::setLogLevel(int n) {
	if (n > 0 && mylog==NULL) {	// open if not opened
		eprintf("%s logfile open for loglevel %d\n", name, n);
		mylog = new logfile_t(name);
		mylog->openfile();
	}
	if (n <= 0 && mylog != NULL) {	// close if not closed
		delete(mylog);
		mylog = NULL;
	}
	return (loglevel=n);
}

int abo::logPacket(const packet_t& packet) {
	if (mylog == NULL) {
		eprintf("%s: ERROR: print to unopened logfile for %s!\n", __FILE__, name);
		return -1;
	}
	if (!mylog->isOK() ) {
		eprintf("%s: ERROR: print to bad logfile for %s!\n", __FILE__, name);
		return -1;
	}
	return mylog->print("%1$.*2$s", packet.data(), packet.datalen());
}

void abo::setFlag(unsigned int add) { flags |=  add; }
void abo::setFlags(unsigned int to) { flags =  to; }
void abo::delFlag(unsigned int del) { flags &= ~del; }
unsigned int abo::getFlags() { return flags; }

int abo::index() {
	return myindex;
/*	for (int i=0; i<noabos; i++) {
		if (list[i] == this) return i;
	}
	eprintf("%s:%d: Internal error. Check list[] entries!\n");
	return -1;	// THIS MUST NEVER HAPPEN!
	*/
}

void abo::printAbos() {
	printf("%d current submission%s%s\n", noabos, (noabos==1?"":"s"),(noabos>0?":":""));
	for (int i=0; i<noabos; i++) {
		printf("  -> %s (last send: %ld seconds ago)\n", list[i]->name, time(NULL)-list[i]->lastrecv);
		for (int a=0; a<list[i]->Suppliers(); a++) {
			printf("     supplied by   %s\n", list[i]->supplier[a]->addrString());
		}
		for (int a=0; a<list[i]->Subscribers(); a++) {
			printf("     subscribed by %s\n", list[i]->subscriber[a]->addrString());
		}
	}
}
