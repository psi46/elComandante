/**
 * \file thread_t.cpp
 * \author Dennis Terhorst
 * \date Wed Oct  7 15:25:09 CEST 2009
 */
#include "thread_t.h"

#include <iostream>
using std::cerr;
#ifdef NDEBUG
#	define cerr if (0) cerr
#endif

thread_t::thread_t(thread_t* Parent, thread_context_t* Context) {
	cerr << "NEW thread_t() " << this << std::endl;
	parent = Parent;
	if (parent != NULL) {
		parent->subthreads.push_back(this);
	}
	context = Context;
	mythread_status = STOPPED;
}

thread_t::~thread_t() throw() {
	cerr << "~thread_t() " << this << " begin" << std::endl;
	// FIXME: call abort() here?
	if ( mythread_status != STOPPED ) this->wait();

	// FIXME: do mutex locking here?

	// BEGIN FIXME: action_context specific stuff
	//
	// cleanup contexts parents (as required by action_context())
	while (context->RefCount()==0) { 	// no delete if context is ref'd elsewhere
		thread_context_t* newroot = context->Parent();	// get parent pointer
		if (newroot==NULL) {
			cerr << "~thread_t(): do not want to delete root context. leaving." << std::endl;
			break;	// finished if this had no more parent (leaving root context undeleted)
		}
		try {
			cerr << "~thread_t(): try delete context" << std::endl;
			delete context;			// now its safe to delete my context
			cerr << "~thread_t(): deleted context" << std::endl;
		} catch (general_exception &e) {
			cerr << "caught exception in ~thread_t(): " << e.what() << std::endl;
			break;
		}
		if (newroot==NULL) {
			cerr << "~thread_t(): finished deleting contexts" << std::endl;
			break;	// finished if this had no more parent (leaving root context undeleted)
		}
		context = newroot;		// else check if parent isn't needed anymore
	}
	//
	// END   FIXME: action_context specific stuff
	
	if (parent != NULL) { // unregister this from parent thread_t
		cerr << "~thread_t(): " << this << " (tid" << TID() << ") unregister from parent thread" << std::endl;
		parent->subthreads.erase(std::find(parent->subthreads.begin(), parent->subthreads.end(), this));
		//parent->refcount--;
	}
	cerr << "~thread_t() "<<this<<" end" << std::endl;
}


const char* thread_t::status() const throw() {
	switch (mythread_status) {
	case RUNNING: return "running";
	case ZOMBIE:  return "zombie";
	case STOPPED: return "stopped";
	}
	cerr << "thread_t::status(): Unknown status " << mythread_status << std::endl;
	return "ERROR";
}

/** \brief abort() conetxt and all sub-contexts
 *
 * This effectively brings this thread to it's end, exiting thread().
 * thread_t is in ZOMBIE state afterwards.
 */
void thread_t::abort() throw() {
	mutex.lock();
	context->abort();	// kill my threads context
	for (subthreads_t::iterator sth  = subthreads.begin();
				    sth != subthreads.end();
				  ++sth) {
		(*sth)->abort();	// ask sub-thread_ts to kill their threads.
	}
	mutex.unlock();
}

/** \brief create a thread
 *
 * This function returns immediately. The run() function of the
 * constructors parameter is called in a separate thread. 
 */
void thread_t::run() throw(errno_exception<EEXIST>) {
	cerr << "void thread_t::run() throw(errno_exception<EEXIST>) {" << std::endl;
//	cerr << "thread_t::run(): STATUS " << status() << std::endl;
	if ( mythread_status != STOPPED ) throw(errno_exception<EEXIST>("there is a thread running or not wait()ed for"));
//	cerr << "thread_t::run(): STATUS " << status() << " run() called" << std::endl;
	if ( pthread_create(&mythread, NULL, thread, this) != 0 ) {
		cerr << "COULD NOT CREATE THREAD: " << strerror(errno) << std::endl;
		return;
	}
	cerr << "void thread_t::run() throw(errno_exception<EEXIST>) }" << std::endl;
//	cerr << "thread_t::run(): STATUS " << status() << " " << std::endl;
}

void thread_t::wait() {
	cerr << "wait() for " << this << "(tid" << TID()<< ") called by tid " << thread_t::gettid() << "(pid" << getpid() << ")" << std::endl;
	if ( mythread_status == STOPPED ) {  cerr << "wait() for stopped " << this << ". done" << std::endl; return; }
	int waitret = pthread_join(mythread, NULL);
	switch (waitret) {
		case 0:
			cerr << "wait() for " << this << " successfull." << std::endl; break;
		case ESRCH:
			cerr << "wait() for " << this << " failed with ESRCH!" << std::endl; break;
		case EINVAL:
			cerr << "wait() for " << this << " failed with EINVAL!" << std::endl; break;
		case EDEADLK:
			cerr << "wait() for " << this << " failed with EDEADLK!" << std::endl; break;
		default:
			cerr << "wait() for " << this << " failed with return value "<< waitret <<"." << std::endl;
	}
	for (subthreads_t::iterator subthread = subthreads.begin(); subthread != subthreads.end(); ++subthread)
		(*subthread)->wait(); // FIXME: either wait, or give responsibility to parent...
	mythread_status = STOPPED;
	cerr << "wait() for " << this << " THREAD " << TID() << " STOPPED" << std::endl;
}

// maintanance and debug stuff
//static
pid_t thread_t::gettid() throw() { return syscall(SYS_gettid); }

pid_t thread_t::TID() const throw() { return tid; }

void thread_t::showTree(std::string pretext) throw() {
	mutex.lock();
	std::cout << pretext << "thread_t @ " << this << "\t(tid=" << TID() << ")" << std::endl;
	for (subthreads_t::iterator subthread = subthreads.begin(); subthread != subthreads.end(); ++subthread) {
		std::cout << pretext << "\tsubthread @ " << (*subthread) << ":" << std::endl;
		(*subthread)->showTree(pretext+"\t");
	}
	mutex.unlock();
}

//static
void thread_t::cleanup(void* thread_t_ptr) {
	thread_t* threadinfo = (thread_t*)thread_t_ptr;
	delete threadinfo;
}

/**
 * \brief the actual thread start routine (GLOBAL Function)
 *
 * this static fumction used as start_routine parameter for the pthread_create call in thread_t::run().
 */
//static
void* thread_t::thread(void* thread_t_ptr) {
	cerr << "void* thread_t::thread(void* thread_t_ptr) {" << std::endl;
	thread_t* threadinfo = (thread_t*)thread_t_ptr;
	threadinfo->mutex.lock();	// [
	threadinfo->mythread_status = thread_t::RUNNING;
	threadinfo->tid = syscall(SYS_gettid);

	cerr << "THREAD " << thread_t::gettid() << " STARTED" << std::endl; //:" is " << threadinfo->context->Name() << endl;
	pthread_cleanup_push(cleanup, threadinfo); // { MUST (!) be pop()ed
	threadinfo->mutex.unlock();	// ]
	
	(void)threadinfo->context->run();
	cerr << "THREAD " << thread_t::gettid() << " ZOMBIE" << std::endl; //:" is " << threadinfo->context->Name() << endl;
	threadinfo->mutex.lock();	// [
	threadinfo->mythread_status = thread_t::ZOMBIE;
	threadinfo->mutex.unlock();	// ]
	pthread_cleanup_pop(1);	// } push thread_t::cleanup
	cerr << "void* thread_t::thread(void* thread_t_ptr) }" << std::endl;
	return NULL;		// return NULL; will not call pthread_cleanup_push()ed handlers
	//pthread_exit(NULL);	// pthread_exit() will call cleanup handlers, but is only usefull between pop&push of the handler
}

