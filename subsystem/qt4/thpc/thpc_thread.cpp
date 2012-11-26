#include "thpc_thread.h"

thpc_thread::thpc_thread(const char* abo){
	me = new sclient(SVR_ADDR, SVR_PORT);
	me -> subscribe(abo);
	wantexit = 0;
}
thpc_thread::~thpc_thread(){
	me->terminate();
}

void thpc_thread::run(){
	//sclient me = sclient(SVR_ADDR, SVR_PORT);
	me->setid("QT_thpc");

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
