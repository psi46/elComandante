/**
 * \file main.cpp
 * \author Dennis Terhorst
 * \date Tue Oct  6 12:08:39 CEST 2009
 */
#include <iostream>
#include <pthread.h>
#include "action_context.h"
#include <subsystem/selectable_sclient.h>
#include <subsystem/daemon.h>
#include <stdlib.h>	// for exit()

using namespace std;


#include <string.h>	// for strerror()
#include <errno.h>

#include "globals.h"
#include "thread_t.h"

#include "sclient_thread.h"


void sig_handler(int sig) {
	if ( global.scptr != NULL ) {
		global.scptr->printf("subscript: received signal %d\n", sig);
	} else {
		cout << "received signal " << sig << endl;
	}
	global.context->abort();
}


#include <fstream>
int main(int argc, char* argv[])
{
	catch_signals(sig_handler);

	cin >> noskipws;
	global.main_is = &cin;
	if (argc > 1) {
		string filename = argv[1];
		global.main_is = new ifstream(filename.c_str());
		if ((!global.main_is->good()) | global.main_is->fail() ) {
			cerr << "problem with file " << filename << endl;
			exit(1);
		}
		*(global.main_is) >> noskipws;
	}

	global.context = new action_context(NULL, global.main_is);

	// sclient connection
//	pthread_t scthread;
	sclient_thread subclient_run;	// sc run class
	thread_t* scthread = new thread_t(NULL, &subclient_run);
//	(void)pthread_create(&scthread, NULL, sclient_thread, &context);
	scthread->run();
	int i=4;
	cout << "Waiting for sclient connection to come up...   " << flush;
	usleep(100);
	while (global.scptr == NULL && i--) {
		cout << "\b" << i << flush;
		sleep(1);
	}
	cout << endl;
	if (global.scptr == NULL) {
		cout << "sclient connection could not be established. exit." << endl;
		exit(1);
	}
	global.context->scptr = (sclient_selectable*)(global.scptr);	// FIXME: volatile!

	cout << "sclient connection registered in context." << endl;

	// parser
	thread_t* thread = new thread_t(NULL, global.context);
	global.context->setThread(thread);	// tell context where it belongs to
	thread->run();	// parallel call
	thread->wait();	// wait for parallel call to exit

	cout << "Waiting for sclient thread to join..." << endl;
	//(void)pthread_join(scthread, NULL);
	global.scptr = NULL;
	scthread->wait();
	cout << "okay. exit." << endl;
	return global.context->result; //is not destroyed by ~thread_t; because parent==NULL
}
