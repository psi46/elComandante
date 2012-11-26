
#include <iostream>
#include <fstream>
#include <unistd.h>	// sleep
#include <pthread.h>
#include <subsystem/daemon.h>
using std::endl;


struct thread_parms {
	int wantexit;	// main incremented (to exit)
	int counter;	// thread incremented
	std::ostream* out;	// main governed pointer
};

void* mythread(void* parms) {
	struct thread_parms *parm = (struct thread_parms *)parms;
	std::ostream &out = *(parm->out);
	for (; (! parm->wantexit) && (parm->counter < 100); (parm->counter)++) {
		out << "Thread:\t" << parm->counter << endl;
		sleep(1);
	}
	return 0;
}


int main(void) {
	std::ofstream out("output.txt", std::ios_base::out );
	//std::ostream& out = std::cout;
	
	thread_parms parms;
	parms.counter=0;
	parms.wantexit=0;
	parms.out = &out;

	daemonize_me(); // threads die upon daemonization, so do this first

	pthread_t thread;
	(void)pthread_create(&thread, NULL, mythread, &parms);

	for (int i=0; i<100; i++) {
		out << "Main:\thallo " << i << endl;
		out << "Main:\tparms->counter = " << parms.counter << endl;
		sleep(1);
	}

	parms.wantexit++;
	out << "waiting for thread to join" << endl;
	(void)pthread_join(thread, NULL);
	out << "okay. exit." << endl;
	

	return 0;
}

