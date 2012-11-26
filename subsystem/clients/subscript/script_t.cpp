/*
 * script_t.cpp
 *
 * Dennis Terhorst <terhorst@physik.rwth-aachen.de>
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>	// snprintf
#include "error.h"
#include <unistd.h>	// usleep
#include <stdlib.h>	// abort

#include <subsystem/signalnames.h>
#include "script_t.h"

#define SCLIENT_OUTPUT

#ifdef SCLIENT_OUTPUT
#  define SUBSCRIPT_INFO		"/subscript/info"
#  define SUBSCRIPT_CONTROL	"/subscript/control"
#  ifdef eprintf
#    undef eprintf
#  endif
#  ifdef eperror
#    undef eperror
#  endif
#  include <string.h>
#  include <errno.h>
#  define eprintf(format, args...) meptr->aprintf(SUBSCRIPT_CONTROL, format , ##args)
#  define eperror(format, args...) meptr->aprintf(SUBSCRIPT_CONTROL, format ": %s\n", ##args, strerror(errno))
#  include <subsystem/sclient.h>
extern sclient* meptr;
#endif


script_t::script_t(const char* Name, const char* Prog, int mode) {
	//eprintf("new script_t(\"%s\", \"%s\");\n", Name, Prog);
	runmode = mode;
	strncpy(name, Name, MAX_SCRIPTNAMELEN);
	strncpy(prog, Prog, MAX_SCRIPTPROGLEN);
	pid=-1;
	reset();
	//printStatus();
}

script_t::~script_t() {
	//eprintf("deleting script_t:\n");
	//printStatus();
	terminate();
}

int script_t::terminate() {
	int t = SCRIPT_KILL_TIMEOUT;
	if (pid > 0) {
		if (SCRIPT_VERBOSE) eprintf("sending SIGTERM to child process group pgid %d\n", pid);
		if (kill(-pid, SIGTERM) < 0) {
			eperror("ERROR: Could not signal child process group pgid %d", pid);
			return -1;
		}
		if (SCRIPT_VERBOSE && !isfinished()) eprintf("waiting for child pid %d to terminate...\n", pid);
		t = SCRIPT_KILL_TIMEOUT;
		while (!isfinished() && t>0) {
			usleep(1000);
			justTerminated();
			t--;
		}
		if ( !isfinished() ) {
			eprintf("WARNING: child pid %d does not exit. Sending SIGKILL.", pid);
			if ( kill(-pid, SIGKILL) < 0) {
				eperror("ERROR: Could not kill child process pid %d", pid);
				return -1;
			}
			if (SCRIPT_VERBOSE) eprintf("waiting for child pid %d to die...\n", pid);
			t = SCRIPT_KILL_TIMEOUT;
			while (!isfinished() && t>0) {
				usleep(1000);
				justTerminated();
				t--;
			}
			if ( !isfinished() ) {
				eprintf("ERROR: could not terminate child process %d!\n", pid);
			}
		}
	}
	return 0;
}


void script_t::reset() {
	if ( pid > 0 ) {
		eprintf("%s:%d: WARNING: script_t::reset() called with valid pid (%d)! Call terminate() first!\n",
			__FILE__,__LINE__, pid);
	}
	pid = -1;
	error = 0;
	exitstatus = 0;
	status = SCRIPT_INIT;
}


bool script_t::operator==(script_t& other) const {
	if (strcmp(name, other.name)==0) { // && strcmp(prog, other.prog) == 0) {
		return true;
	}
	return false;
}


bool script_t::isRunning() const {
	return (status == SCRIPT_RUNNING);
}


bool script_t::isfinished() const {
	if (status == SCRIPT_ENDED || status == SCRIPT_ERROR)
		return true;
	return false;
}


bool script_t::justTerminated() {
	int ret;
	if (status != SCRIPT_RUNNING) return false;
	ret = waitpid(pid, &exitstatus, WNOHANG);
	if        (ret < 0 ) {
		// waidpid error
		eperror("%s:%d: ERROR: waitpid() failed!", __FILE__, __LINE__);
		status = SCRIPT_ERROR;
	} else if (ret == 0) {
		if (SCRIPT_VERBOSE) eprintf("pid %d still running\n", pid);
		// process still running
	} else {
		// process just ended.
		if (SCRIPT_VERBOSE) {
			eprintf("child pid %d terminated. ", pid);
			print_exitstatus();
		}
		pid = -1;
		if (wasNormalExit())
			status = SCRIPT_ENDED;
		else
			status = SCRIPT_ERROR;
		return true;
	}
	return false;
}


int script_t::setRunmode(int mode) { return (runmode = mode); } // note maybe do some checks

int script_t::getRunmode() const { return runmode; }

int script_t::getPID() const { return pid; }

int script_t::getExitStatus() const { return exitstatus; } // do not confuse with status!

int script_t::getStatus() const { return status; }


const char* script_t::getStatus_string() const {
	switch (status) {
	case SCRIPT_INIT:	return "initializing";
	case SCRIPT_RUNNING:	return "running";
	case SCRIPT_STOPPED:	return "STOPPED";
	case SCRIPT_ENDED:	return "finished";
	case SCRIPT_ERROR:	return "ERROR";
	}
	
	eprintf("%s:%d: undefinded status!\n", __LINE__, __FILE__);
	return "undefined";		
}

const char* script_t::getName() const {
	return (const char*)name;
}

int script_t::operator()(char* const argv[]) {
	return run(argv);
}


int script_t::run(char* const argv[]) {
	if (status != SCRIPT_INIT) return -1;

	
	switch (runmode) {
	case SCRIPT_RUNMODE_SCRIPT:
		run_script(argv);
		break;
	case SCRIPT_RUNMODE_MODULE:
		run_module(argv);
		break;
	default:
		// (unknown runmode)
		eprintf("%s:%d: ERROR: unknown run mode for script %s\n", __FILE__, __LINE__, name);
		return -1;
	}

	return (status == SCRIPT_RUNNING);
}


// run the given prog with stdin and stdout
// redirected to a bidirectional subclient.
int script_t::run_script(char* const argv[]) {	/////////////////////////////////////////////// RUN SCRIPT
	eprintf("%s:%d: FIXME: SCRIPT_RUN_SCRIPT not jet implemented.\n", __FILE__, __LINE__);
}


// run the given prog as self-contained child process
int script_t::run_module(char* const argv[]) {	/////////////////////////////////////////////// RUN MODULE

#ifndef SCRIPT_USE_SYSTEM_COMMAND
	//
	// block sigCHLD in case exec fails. It would cause the async execution of the parents
	// signal handler
	//
	//int oldsigmask = siggetmask();
	//sigblock(sigmask(SIGCHLD));
	sigset_t oldsigset;
	sigset_t myblocks;
	sigemptyset(&myblocks);
	sigaddset(&myblocks, SIGCHLD);
	sigprocmask(SIG_BLOCK, &myblocks, &oldsigset);	// add SIGCHLD to blocked signals
	
	// fork separates processes with copy-on-write mem copy
	// vfork separates processes with child running on
	//  parent stack, but abort causes both procs to die.
	pid_t newpid = fork();			// FORK

	switch (newpid) {
	case -1: // error
		error = errno;
		status = SCRIPT_ERROR;
		eperror("%s:%d: ERROR: could not vfork()\n", __FILE__, __LINE__);
		break;
	case 0: // child
		sigprocmask(SIG_SETMASK, &oldsigset, NULL); // restore old sigmask
		setsid();	// create new process group
		if (argv == NULL) {
			if (execl(prog, NULL) < 0) {	// EXEC
				//error = errno;
				//status = SCRIPT_ERROR;
				eperror("%s:%d: ERROR: could not exec \"%s\"", __FILE__, __LINE__, prog);
			}
		} else {
			if (execv(prog, argv) < 0) {	// EXEC
				//error = errno;
				//status = SCRIPT_ERROR;
				eperror("%s:%d: ERROR: could not exec \"%s\"", __FILE__, __LINE__, prog);
			}
		}
		abort(); // abort child if exec failed
		//_exit(1);
		break;
	default: // parent
		pid = newpid;
		status = SCRIPT_RUNNING;
		if (SCRIPT_VERBOSE) eprintf("child pid %d running\n", pid);
	}
	// ALTERNATIVE: int system()
	sigprocmask(SIG_SETMASK, &oldsigset, NULL); // restore old sigmask
#else
	sigset_t oldsigset;
	sigset_t myblocks;
	sigemptyset(&myblocks);
	sigaddset(&myblocks, SIGCHLD);
	sigprocmask(SIG_BLOCK, &myblocks, &oldsigset);	// add SIGCHLD to blocked signals
	eprintf("%s:%d: HERE\n", __FILE__, __LINE__);
	char cmd[1024];
	char parm[1024];
	snprintf(cmd, 1024, "%s ", prog);
	if (argv != NULL) {
		int i=0;
		while (argv[i]!=NULL) {
			snprintf(parm, 1024, "%s ", argv[i]);
			strncat(cmd, parm, 1024-strlen(cmd));
		}
	}
	eprintf("wanting to start \"%s\"\n", cmd);
	switch (exitstatus = system(cmd)) {
	case -1:
		eprintf("ERROR: system(\"%s\") returned %d\n", cmd, exitstatus);
		print_exitstatus();
		status = SCRIPT_ERROR;
		break;
	default:
		eprintf("system(\"%s\") returned %d\n", cmd, exitstatus);
		print_exitstatus();
		status = SCRIPT_RUNNING;	
	}
	sigprocmask(SIG_SETMASK, &oldsigset, NULL); // restore old sigmask
#endif
} // end run_module

bool script_t::wasNormalExit() {
	if (WIFEXITED(exitstatus)) {
		return true;
	}
	return false;
}

char script_t::runmodechar() const {
	switch (runmode) {
	case SCRIPT_RUNMODE_SCRIPT:	return 'S';
	case SCRIPT_RUNMODE_MODULE:	return 'M';
	}
	return '?';
}

const char* script_t::status_string() const {
	switch (status) {
	case SCRIPT_INIT:	return "I   ";
	case SCRIPT_RUNNING:	return " R  ";
	case SCRIPT_ENDED:	return "  D ";
	case SCRIPT_STOPPED:	return " S  ";
	case SCRIPT_ERROR:	return " ERR";
	}
	return "????";
}

const char* script_t::Status_head_string() {
	return "RM STAT EXIT   PID                 NAME PROG";
}

const char* script_t::Status_string() const {
	static char stat[120];
	if (pid>0) {
		snprintf(stat, 120, "%c  %s %4d %5d %20s \"%s\"",
			runmodechar(), status_string(), WEXITSTATUS(exitstatus), pid, name, prog);
	} else { 
		snprintf(stat, 120, "%c  %s %4d       %20s \"%s\"",
			runmodechar(), status_string(), WEXITSTATUS(exitstatus),  name, prog);
	}

	return stat;
}

void script_t::print_exitstatus() {
	if (WIFEXITED(exitstatus)) {
		eprintf("normal exit with status %d.\n", WEXITSTATUS(exitstatus));
	} else if (WIFSIGNALED(exitstatus)) {
		eprintf("abnormal exit due to signal %s(%d).\n", SIGNAME[WTERMSIG(exitstatus)], WTERMSIG(exitstatus));
#ifdef WCOREDUMP
		if (WCOREDUMP(exitstatus)) {
			eprintf("core dumped.\n");
		}
#endif
	} else if (WIFSTOPPED(exitstatus)) {
		eprintf("process stopped by signal %s(%d)\n", SIGNAME[WSTOPSIG(exitstatus)], WSTOPSIG(exitstatus));
	} else {
		eprintf("Unknown exit status: %d\n", exitstatus);
	}
	return;
}


