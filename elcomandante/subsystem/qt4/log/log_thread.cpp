/**
 * Jochen Steinmann <jochen.steinmann@rwth-aachen.de>
 */
#include "log_thread.h"

log_thread::log_thread(){
	me = new sclient(SVR_ADDR, SVR_PORT);
	wantexit = 1;
}
log_thread::~log_thread(){
	me->terminate();
}
sclient* log_thread::getsclient(){
	return me;
}

void log_thread::run(){
	//sclient me = sclient(SVR_ADDR, SVR_PORT);
	me->setid("QT_log");
	wantexit = 0;
	while (wantexit == 0) {
		// read packet
		//printf("waiting for packet\n");
		if ( me->recvpacket(packet) < 0 ) {
			printf("got error\n");
			continue;
		}
		//packet.print();
		//printf("%1$.*2$s", packet.data(), packet.datalen());
		emit received(packet.data(), packet.datalen());
	}
}
