#include "uni_thread.h"

uni_thread::uni_thread(){
	me = new sclient(SVR_ADDR, SVR_PORT);
	wantexit = 1;
}
uni_thread::~uni_thread(){
	me->terminate();
}
sclient* uni_thread::getsclient(){
	return me;
}

void uni_thread::run(){
	//sclient me = sclient(SVR_ADDR, SVR_PORT);
	me->setid("QT_uni");
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
		emit received(atof(packet.data()));

	}
}
