/*
 * \file script_t.h
 *
 * \author Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 * \date 12 Mar 2008
 */
#ifndef SUBSCRIPT_H
#define SUBSCRIPT_H

#include <sys/types.h>	// pid_t

#define MAX_SCRIPTNAMELEN	128
#define MAX_SCRIPTPROGLEN	256
#define SCRIPT_INIT	0
#define SCRIPT_RUNNING	1
#define SCRIPT_STOPPED	2
#define SCRIPT_ENDED	3
#define SCRIPT_ERROR	4

#define SCRIPT_RUNMODE_SCRIPT	0
#define SCRIPT_RUNMODE_MODULE	1
#define SCRIPT_DEFAULT_RUNMODE	SCRIPT_RUNMODE_MODULE

// destructor kill timeout (milliseconds)
#define SCRIPT_KILL_TIMEOUT	10

#define SCRIPT_VERBOSE	1

// run_module() method selection. use own fork()-exec(), or system()
//#define SCRIPT_USE_SYSTEM_COMMAND

/**
 * \brief child process management
 *
 * Using this class a sub-process can be created and monitored
 */
class script_t {
private:
	pid_t	pid;
	char	name[MAX_SCRIPTNAMELEN];
	char	prog[MAX_SCRIPTPROGLEN];
	int	runmode;	// start as script or module
	int	status;		// status of script (SCRIPT_INIT, SCRIPT_RUNNING, ...)
	int	exitstatus;	// exitstatus of child process
	int	error;		// errno of system calls

public: // *structors
	script_t(const char* Name, const char* Prog, int mode = SCRIPT_DEFAULT_RUNMODE) ;
	~script_t() ;

public:	// operators
	bool operator==(script_t& other) const ;
	int operator()(char* const argv[]=NULL) ;

public:	// settings + functions
	int setRunmode(int mode) ; // note maybe do some checks
	int getRunmode() const ;
	void reset() ;			// reset script properties
	int run(char* const argv[]) ;	// start script
	int terminate();		// send kill signal
	// int stop();		// FIXME
	// int continue();	// FIXME
	// int kill();		// FIXME

private:
	// run the given prog with stdin and stdout
	// redirected to a bidirectional subclient.
	int run_script(char* const argv[]) ;

	// run the given prog as self-contained child process
	int run_module(char* const argv[]) ;

	bool wasNormalExit() ;
	
	char runmodechar() const ;

	const char* status_string() const ;


public:	// informational
	bool isRunning() const ;
	bool isfinished() const ;
	bool justTerminated() ;		// with waidpid() call
	int getPID() const ;
	int getExitStatus() const ; // do not confuse with status!
	int getStatus() const ;				// state of script
	const char* getStatus_string() const ;		// state of script
	static const char* Status_head_string() ;	// ls type output (head)
	const char* Status_string() const ;		// ls type output
	const char* getName() const;
private:
	void print_exitstatus() ;
};


#endif
