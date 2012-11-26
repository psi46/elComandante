#include <iostream>
#include <pthread.h>
#include <unistd.h>
using namespace std;

void* thread(void*);


class mutex_t {
	pthread_mutex_t	mutex;
public:
	mutex_t(const pthread_mutexattr_t * attr=NULL) {
		if ( pthread_mutex_init(&mutex, attr) != 0 ) throw("could not initialize mutex");
	}
	virtual ~mutex_t() { pthread_mutex_destroy(&mutex); }
	int lock() { return pthread_mutex_lock(&mutex); }
	int unlock() { return pthread_mutex_unlock(&mutex); }
};

struct global_t {
	mutex_t cout_mux;
} global;



class action_context_t {
public:
	void run() {
		int ret=10;
		while ( ret > 0 ) {
			global.cout_mux.lock();
			cout << "thread " << ret << endl;
			global.cout_mux.unlock();
			sleep(1);
			ret--;
		}
		return;
	}

};

#include <string.h>	// for strerror
#include <errno.h>	// for errno
template <class runclass_t>
class thread_t {
	//typedef action_context_t runclass_t;	// this class just needs to have a run() function which will be called from the thread
	typedef enum {RUNNING, ZOMBIE, STOPPED} pthread_status_t;

	pthread_t mythread;
	pthread_status_t mythread_status;
	runclass_t* context;
public:
	thread_t(action_context_t* Context) {
		context = Context;
		mythread_status = STOPPED;
	}
	virtual ~thread_t() {
		if ( mythread_status != STOPPED ) wait();
	}
	const char* status() {
		switch (mythread_status) {
		case RUNNING: return "running";
		case ZOMBIE:  return "zombie";
		case STOPPED: return "stopped";
		}
		return "ERROR";
	}

	void run() {
		if ( pthread_create(&mythread, NULL, thread, this) != 0 ) {
			cerr << "COULD NOT CREATE THREAD: " << strerror(errno) << endl;
			return;
		}
		mythread_status = RUNNING;
	}

	void wait() {
		(void)pthread_join(mythread, NULL);
		mythread_status = STOPPED;
	}
private:
	static void* thread(void* Context) {
		thread_t* context = (thread_t*)Context;
		context->context->run();
		context->mythread_status = thread_t::ZOMBIE;
		return NULL;
	}
};


int main(void) {
	action_context_t context;
	thread_t<action_context_t> thread(&context);

	
	cout << "main" << "\t thread stat:" << thread.status() << endl;
	cout << "main" << "\t starting thread" << endl;
	context.run();
	cout << "main" << "\t thread stat:" << thread.status() << endl;
	thread.run();
	cout << "main" << "\t thread stat:" << thread.status() << endl;

	int ret=5;
	while ( ret > 0 ) {
		global.cout_mux.lock();
		cout << "main " << ret << "\t thread stat:" << thread.status() << endl;
		global.cout_mux.unlock();
		sleep(1);
		ret--;
	}

	cout << "main" << "\t thread stat:" << thread.status() << endl;
//	cout << "main" << "\t waiting for thread..." << endl;
//	thread.wait();
//	cout << "main" << "\t thread stat:" << thread.status() << endl;
	cout << "exit okay." << endl;
	return 0;
}
