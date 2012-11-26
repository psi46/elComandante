/**
 * \file thread_t.h
 * \author Dennis Terhorst
 * \date Mon Nov  9 21:25:45 CET 2009
 */
#ifndef THREAD_T_H
#define THREAD_T_H

#include <string.h>	// for strerror
#include <errno.h>	// for errno
#include <iostream>	// for cerr
//#include <sys/types.h>	// for gettid() does not work
#include <unistd.h>	// for syscall()
#include <sys/syscall.h>// for syscall()
#include <sys/types.h>	// for syscall()
#include <pthread.h>
#include <vector>
#include <algorithm>
#include "exceptions.h"

/** \brief pthread_mutex_* wrapper class
 *
 * This class wrapper for the pthread_mutex_* functions probably slows down the
 * calls, but eases use.
 */
class mutex_t {
	pthread_mutex_t	mutex;
public:
	mutex_t(const pthread_mutexattr_t * attr=NULL) {
		switch ( pthread_mutex_init(&mutex, attr) ) {
		case 0:		break; // success
		case EINVAL:	throw(errno_exception<EINVAL>("invalid mutex attributes"));
		case EAGAIN:	throw(errno_exception<EAGAIN>("lacked the necessary resources (other than memory) to initialize mutex"));
		case ENOMEM:	throw(errno_exception<ENOMEM>("insufficient memory to initialize the mutex"));
		case EPERM:	throw(errno_exception<EPERM>("no permission to initialize the mutex"));
		case EBUSY:	throw(errno_exception<EBUSY>("attempt to reinitialize initialized mutex"));
		default:	throw(general_exception("could not initialize mutex (unknown return value)"));
		}
	}
	virtual ~mutex_t() {
		switch ( pthread_mutex_destroy(&mutex) ) {
		case 0:		break; // success
		case EINVAL:	throw(errno_exception<EINVAL>("attempt to destroy uninitialized mutex"));
		case EBUSY:	throw(errno_exception<EBUSY>("attempt to destroy locked mutex"));
		default:	throw(general_exception("could not destroy mutex (unknown return value)"));
		}
	}
	int lock() { return pthread_mutex_lock(&mutex); }
	int unlock() { return pthread_mutex_unlock(&mutex); }
};

/** \brief context class to run a separate thread in/with/from
 *
 * A thread_t will call theses functions to start and stop thread execution.
 * To start a thread put code in a function called run() inside your context
 * class and derive from this class. Then thread_t will manage the thread.
 */
class thread_context_t {
public:
	virtual ~thread_context_t() { }
	virtual int run()=0;
	virtual void abort()=0;
	virtual thread_context_t* Parent() const { return NULL; }	// needed for action_context cleanup
	virtual int RefCount() const { return 0; }
};

/** \brief pthread_create/join wrapper class
 *
 * the template parameter class just needs to have a run() function which will
 * be called from the thread, and needs an abort() function.
 */
//template <class thread_context_t>
class thread_t {
	typedef enum {RUNNING, ZOMBIE, STOPPED} pthread_status_t;

	pthread_t mythread;			///< posix thread id (opaque handle)
	pid_t tid;				///< linux thread id (similar to pid, check 'ps -mfLC action')
	pthread_status_t mythread_status;	///< status of the corresponding thread
	thread_context_t* context;		///< context associated with this thread
	
	// thread_t tree
	thread_t* parent;
	typedef std::vector< thread_t* > subthreads_t;
	subthreads_t subthreads;	///< sub threads of this context
	// refcount = subthreads.size();
	mutex_t mutex;

public:
	thread_t(thread_t* Parent, thread_context_t* Context);

	virtual ~thread_t() throw();


/*	thread_t* create_subthread(thread_context_t* Context) {
		subthreads.push_back(new thread_t(Context) );
	//	subthreads[subthreads.size()-1]->run();
		return (subthreads[subthreads.size()-1]);
	}*/

	const char* status() const throw();

	/** \brief abort() conetxt and all sub-contexts
	 *
	 * This effectively brings this thread to it's end, exiting thread().
	 * thread_t is in ZOMBIE state afterwards.
	 */
	void abort() throw();

	/** \brief create a thread
	 *
	 * This function returns immediately. The run() function of the
	 * constructors parameter is called in a separate thread. 
	 */
	void run() throw(errno_exception<EEXIST>);

	void wait();

	// maintanance and debug stuff
	static pid_t gettid() throw();
	pid_t TID() const throw();
	void showTree(std::string pretext="") throw();
private:
	static void cleanup(void* thread_t_ptr);

	/**
	 * \brief the actual thread start routine
	 *
	 * this static fumction used as start_routine parameter for the pthread_create call in thread_t::run(). 
	 */
	static void* thread(void* thread_t_ptr);
};


#endif //ndef THREAD_T_H
