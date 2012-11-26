/**
 * sclient_qthread.h
 *
 * Dennis Terhorst
 * Tue Jul 22 23:05:54 CEST 2008
 */
#include "sclient_qthread.h"
#include <sys/select.h>	// select()
#include <subsystem/error.h>	// global debug path

sclient_qthread::sclient_qthread(QObject* parent) : QThread(parent)
{
	abort = false;
	restart = false;
	me = new sclient();
	if (me->isOK()) {
		me->setid("QT_sclient");
		emit connection_established();
	} else {
		emit connection_failed(); 
	}
}

sclient_qthread::~sclient_qthread()
{
    mutex.lock();
    	abort = true;
	me->terminate();
	delete me;
	emit connection_terminated();
	//condition.wakeOne();
    mutex.unlock();
	wait();
}

void sclient_qthread::sendpacket(packet_t Packet)
{
	QMutexLocker locker(&mutex);

	me->sendpacket(Packet);
	//packet = Packet;	// FIXME enable this!
	//hassendpacket = true;
	if (!isRunning()) {
		start(); //LowPriority);
	} else {
		restart = true;
		//condition.wakeOne();
	}
}

const sclient* sclient_qthread::getsclient(){
	return me;
}

void sclient_qthread::run(){
	packet_t packet;

	// select preparations
	fd_set allfds;
	fd_set readfds;
	int maxfd;
	FD_ZERO(&allfds);
	FD_SET(me->getfd(), &allfds);
	maxfd = me->getfd();
	struct timeval timeout;
	int noready;

	forever {
		timeout.tv_sec=0;
		timeout.tv_usec=100000;
		memcpy(&readfds, &allfds, sizeof(fd_set)); //readfds = allfds;

		if (abort)
			return;

		noready = select(maxfd+1, &readfds, NULL, NULL, &timeout);

		if ( noready < 0 ) {		// ERROR
			eperror("%s:%d: select() call failed", __FILE__, __LINE__);
		} else if ( noready == 0 ) {	// TIMEOUT
			// ...boring...
			emit myheartbeat();
			//eprintf("-thread-\n");
		} else {			// DATA
			if ( me->recvpacket(packet) < 0 ) {
				eprintf("%s:%d: recvpacket() error\n",  __FILE__, __LINE__);
				emit receive_error();
				continue;
			}
			emit received(packet);
			//condition.wakeOne(); // FIXME: is this needed?
		}

/*	    mutex.lock();
		if (!restart)
			condition.wait(&mutex);
		restart = false;
	    mutex.unlock();*/
	}
}


