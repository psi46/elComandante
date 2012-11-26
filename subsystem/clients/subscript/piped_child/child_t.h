#ifndef CHILD_T_H
#define CHILD_T_H

#include <signal.h>
#include <time.h>

// values for child_t::status
// (as defined in bits/siginfo.h <- signal.h )
// define CLD_EXITED	1
// define CLD_KILLED	
// define CLD_DUMPED	
// define CLD_TRAPPED	
// define CLD_STOPPED	
// define CLD_CONTINUED
//
// additional values for child_t::status
#ifndef CLD_RUNNING
#  define CLD_RUNNING		0
#endif
#ifndef CLD_UNKNOWN_STATUS
#  define CLD_UNKNOWN_STATUS	-2
#endif

// minimum time between fork calls (exit if child
// process dies to quick and --restart option is
// used) FIXME: this should probably somewhere else
#define MIN_FORK_DELTA_T	1

#define P_W	1	// write end of the pipe
#define P_R	0	// read end of the pipe

#include "selectable_pipe.h"
#define MAX_CHILDREN MAX_SELECTABLE
#ifndef MAX_NAMELEN
#  define MAX_NAMELEN 256
#  warning MAX_NAMELEN redefinition must comply with abo.h
#endif

// info structure for the child process
class child_t {

// STATIC DATA MEMBERS
private:
	static child_t*	child[MAX_CHILDREN];
	static int noc;	// number of children

// STATIC FUNCTIONS
public:
	static child_t* findbyPID(int pid);
	static void sigchld_handler(int sig);
	static void ListChildren();
	static int getnoc();

// PRIVATE DATA MEMBERS
private:
	int thischild;
	pipe_selectable<WRITE>	inp;
	pipe_selectable<READ>	outp;
	pipe_selectable<READ>	errp;
	volatile pid_t	pid;	// child pid
	volatile int	status;
	time_t	last_restart_time;
	char**	argv;
	int	argc;
	char	clientid[MAX_NAMELEN];

	child_t& operator=(child_t& other);	// do not implement! do not use!
// PUBLIC FUNCTIONS
public:
	child_t(int argc, char* argv[], char* ClientID);
	virtual ~child_t();

	int getStatus() const;
	char* ChildStatusString() const;
	pid_t getPID() const;
	pid_t setPID(pid_t newpid);

	pipe_selectable<WRITE>* stdin_pipe();
	pipe_selectable<READ>* stdout_pipe();
	pipe_selectable<READ>* stderr_pipe();

	void PrintInfo();

	pid_t dofork();
	int terminate();
private:
	int child_prefork_init();
	int child_postmortem();
	int child_exec();
	int parent_postfork_init();

};



#endif //ndef CHILD_T_H
