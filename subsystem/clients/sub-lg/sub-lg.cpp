#include <sclient.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


sclient* me;

bool timeout=false;
void sigALRM(int s) {
	timeout=true;
	delete me;
	exit(0);
}

int main(){
	signal(SIGALRM, sigALRM);
	me = new sclient();
	me->setid("sub-lg");
	char buffer[123] = "lg";
	int buflen = 3;
	packet_t p;
	p.type = PKT_MANAGEMENT;
	p.setName("/server/clientinfo");
	p.setData(buffer,buflen);
	//printf("PACKET COMPLETE. SENDING...\n");
	//printf("PACKET: \"%1$.*2$s\"\n", p.data(), p.datalen());
	me->sendpacket(p);

	alarm(2);
	while (me->recvpacket(p)>0) {
		if (timeout) break;
		if (p.datalen() > 0)
			printf("%1$.*2$s", p.data(), p.datalen());
	}
}
