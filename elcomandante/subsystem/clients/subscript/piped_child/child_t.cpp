#include "child_t.h"
#include <unistd.h>	// pipe, dup2, execvp, setsid
#include <stdio.h>	// perror
#include "signal_handling.h"
#include <fcntl.h>	// fnctl ( see FIXME)
#include <stdlib.h>	// abort
#include <string.h>	// strncpy ( see FIXME)

// CLASS STATICS
int child_t::noc = 0;
child_t* child_t::child[MAX_CHILDREN] = {NULL};


// CONSTRUCTOR
child_t::child_t(int Argc, char* Argv[], char* ClientID) {
	thischild = noc;
	child[thischild] = this;
	noc++;
	argc = Argc;
	argv = Argv;	// FIXME: this points out of own space!
	strncpy(clientid, ClientID, MAX_NAMELEN);	// FIXME: we do not want to store this name here
	clientid[MAX_NAMELEN-1]=0;
}

// DESTRUCTOR
child_t::~child_t() {
	child[thischild] = child[noc];
	noc--;
	signal_block(SIGCHLD);
	if (pid >0) { kill(pid, SIGKILL); }	// FIXME: this will cause an unmatched SIGCHLD
	signal_unblock(SIGCHLD);
}

void child_t::PrintInfo() {
	printf("\targc=%d\n", argc);
	for (int i=0; i<argc; i++) printf("\targv[%d]=%s\n", i, argv[i]);
	fflush(stdout);
}


char* child_t::ChildStatusString() const {
	switch (status) {
	case CLD_EXITED: return "CLD_EXITED";
	case CLD_KILLED: return "CLD_KILLED";
	case CLD_DUMPED: return "CLD_DUMPED";
	case CLD_TRAPPED: return "CLD_TRAPPED";
	case CLD_STOPPED: return "CLD_STOPPED";
	case CLD_CONTINUED: return "CLD_CONTINUED";
	case CLD_RUNNING: return "CLD_RUNNING";
	case CLD_UNKNOWN_STATUS: return "CLD_UNKNOWN_STATUS";
	default: return "UNKNOWN_VALUE";
	}
}

pipe_selectable<WRITE>* child_t::stdin_pipe() { return &inp; }
pipe_selectable<READ>* child_t::stdout_pipe() { return &outp; }
pipe_selectable<READ>* child_t::stderr_pipe() { return &errp; }

int child_t::terminate() {
	signal_block(SIGCHLD);
	if ( pid > 0 ) {
		kill(pid, SIGTERM);
		printf("waiting for child termination...\n");
		signal_unblock(SIGCHLD);
		sleep(1);
	}
	signal_unblock(SIGCHLD);
	this->child_postmortem();
	return ( status != CLD_RUNNING );
}

int child_t::child_prefork_init() {

	status = CLD_UNKNOWN_STATUS;

	// PREPARE CHILD
	inp.reopen();
	outp.reopen();
	errp.reopen();

	// prevent fork-loop
	//if (time(NULL)-last_restart_time < MIN_FORK_DELTA_T ) {
	//	fprintf(stderr, "%ld %s > info='child respawn too quick -- aborting.'\n", time(NULL), clientid);
	//	return -1;
	//}

	// catch SIGCHLD
	signal_block(SIGCHLD);	// do not interrupt parent to early
	if ( signal_handler(SIGCHLD, sigchld_handler) < 0) {
		fprintf(stderr, "ERROR: Could not retister SIGCHLD handler. Bailing out.\n");
		return 1;
	}

	return 0;
}

int child_t::parent_postfork_init() {	// this is only executed by the parent
	// close the pipes other ends.
	inp.close_read();
	outp.close_write();
	errp.close_write();
	//fprintf(stderr, "parent: child stdin is connected to fd%d for %d\n", inp.getfd(), inp.getchecks());
	//fprintf(stderr, "parent: child stdout is connected to fd%d for %d\n", outp.getfd(), outp.getchecks());
	//fprintf(stderr, "parent: child stderr is connected to fd%d for %d\n", errp.getfd(), errp.getchecks());
	status = CLD_RUNNING;
	signal_unblock(SIGCHLD);	// UNBLOCK
	return 0;	
}


// close any leftover fds
int child_t::child_postmortem() {
	inp.close_all(); outp.close_all(); errp.close_all();
	return 0;
}

int child_t::getStatus() const { return status; }
pid_t child_t::getPID() const { return pid; }
pid_t child_t::setPID(pid_t newpid) { return (pid=newpid); }

pid_t child_t::dofork() {
	if ( this->child_prefork_init() < 0 )
		{ fprintf(stderr, "ERROR: could not initialize child (prefork)\n"); return -1; }

	time(&(last_restart_time));
	if ( (this->setPID(fork()) ) < 0 ) {
		perror("could not fork");
		this->child_postmortem();
		return pid;
	}
	if ( 0 == this->getPID() ) {
		// CHILD //////////////////////////////////////////////////////////////////////////////////////
		this->child_exec();
		perror("ERROR: child_exec failed");
		kill(getppid(), SIGCHLD);	// notify parent
		abort(); //return SIGABRT;
	}
	// PARENT //////////////////////////////////////////////////////////////////////////////////////
	// close the pipes other ends and prepare to receive SIGCHLD (unblock)
	this->parent_postfork_init();
	return pid;
}

int child_t::child_exec() {
	//printf("child: ppid/pid=%d/%d\n", getppid(), getpid());
	// pipes directed from the child, i.e. childs in[] fd is a read end of the pipe.
	// close the other ends.
	inp.close_write();
	outp.close_read();
	errp.close_read();

	// map pipes to stdio file descriptors
	if ( dup2( inp.rfd(),  STDIN_FILENO  ) < 0 ) { perror("child: dup2 failed"); return 1; };
	if ( dup2( outp.wfd(), STDOUT_FILENO ) < 0 ) { perror("child: dup2 failed"); return 1; };
	if ( dup2( errp.wfd(), STDERR_FILENO ) < 0 ) { perror("child: dup2 failed"); return 1; };

	// this does not hold across execvp()
	if ( setvbuf(stderr, NULL, _IONBF, 0) != 0 ) { perror("child: could not set _IONBF for stderr"); return 1; }
	if ( setvbuf(stdout, NULL, _IONBF, 0) != 0 ) { perror("child: could not set _IONBF for stdout"); return 1; }

	// FIXME: following seems to have no effekt
	//if ( fcntl(STDOUT_FILENO, F_SETFL, O_SYNC) < 0 ) { perror("child: could not set O_SYNC for stdout"); return 1; }

	setsid();	// make own session, to have our own signal scope

	//printf("child-preexec: ppid/pid=%d/%d\n", getppid(), getpid());
	// exec into target program
	// 'status =' is useless before exec
	execvp(argv[0], argv);	// NEVER RETURNS (except on error)
	return -1;
}

#include <sclient.h>	// MAX_PACKETLEN

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#define lineadd(line, args...)	snprintf(&(line[strlen(line)]), MAX_PACKETLEN-strlen(line), ##args)

// GLOBALS from the main
extern sclient* meptr;

child_t* child_t::findbyPID(int pid) {
	for (int i=0; i<noc; i++) {
		if ( child[i]->pid == pid ) return child[i];
	}
	return NULL;
}

void child_t::ListChildren() {
	
	fprintf(stderr, "%d children:\n", noc);
	for (int i=0; i<noc; i++) {
		fprintf(stderr, "child[%d].pid = %d\n", i, child[i]->pid);
	}
}
int child_t::getnoc() { return noc; }

void child_t::sigchld_handler(int sig) {
	siginfo_t siginfo;
	char line[MAX_PACKETLEN]={0};
	fprintf(stderr, "parent: received signal %d.\n", sig);

	siginfo.si_pid=0;					// get wait info
	if ( waitid(P_ALL, 0, &siginfo, WEXITED | WSTOPPED | WCONTINUED | WNOHANG) < 0 ) {
		perror("wait failed");
	}
	if ( siginfo.si_pid == 0 ) {
		printf("warning: wait returned siginfo.si_pid==0.\n");
		return;
	}

	child_t* child = child_t::findbyPID(siginfo.si_pid);	// find corresponding child
	if ( child == NULL ) {
		printf("warning: received SIGCHLD from (pid %d), which is no child!\n", siginfo.si_pid);
		child_t::ListChildren();
		return;
	}

	// update child status
	child->status = siginfo.si_code;
	child->pid = (pid_t)-1;
	line[0]=0;	// clear line				// compose info message
	lineadd(line, "%s > status=%s",  child->clientid, child->ChildStatusString() );
	//lineadd(line, "FIXME > status=%s",  child->ChildStatusString() );	// FIXME: no clientid here
	lineadd(line, ", pid=%d", siginfo.si_pid);
	lineadd(line, ", uid=%d", siginfo.si_uid);
	if (WIFSIGNALED(siginfo.si_status)) {
		lineadd(line, ", termsig=%d", WTERMSIG(siginfo.si_status));
#		ifdef WCOREDUMP
		lineadd(line, ", coredump=%s", WCOREDUMP(siginfo.si_status)?"true":"false");
#		endif
	}
	if (WIFSTOPPED(siginfo.si_status) ) {
		lineadd(line, ", stopsig=%d", WSTOPSIG(siginfo.si_status));
	}
	if (WIFEXITED(siginfo.si_status) ) {
		lineadd(line, ", return_status=%d", WEXITSTATUS(siginfo.si_status));
	}
	if ( meptr != NULL ) {
		(meptr)->printf("%ld %s\n", time(NULL), line);
	} else {
		printf("%s\n", line);
	}
}

