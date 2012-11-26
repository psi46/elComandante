//
// Daemonisierung
// DT May2006
// 

#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h> // for sigaction

#include "daemon.h"

int daemonize_me() {
	pid_t pid;
	if ((pid=fork()) < 0) {
#ifdef DEAMON_SELFDEBUG
		perror("daemon.c: fork()-Fehler");
#endif
		return(-1);
	}
	
	if (pid != 0)	// parent exit
		_exit(0);
	
	setsid();
	chdir(DAEMON_WORKDIR);
	umask(DAEMON_UMASK);
	return 0;
}


int catch_signals(void (*handler)(int)) {
	struct sigaction saio;           // definition of signal action
	// do correct cleanup in most cases
	//atexit(handler);

	sigemptyset(&saio.sa_mask);
	saio.sa_flags = 0;

	#ifndef MAC_OS
		saio.sa_restorer = NULL;
	#endif
	saio.sa_handler = handler;	// given handler to
	sigaction(SIGINT,  &saio, NULL);	// CATCH  SIGINT
	
	sigemptyset(&saio.sa_mask);
	saio.sa_flags = 0;
        #ifndef MAC_OS
                saio.sa_restorer = NULL;
        #endif
	saio.sa_handler = handler;	// given handler to
	sigaction(SIGTERM, &saio, NULL);	// CATCH  SIGTERM

	sigemptyset(&saio.sa_mask);
	saio.sa_flags = 0;
        #ifndef MAC_OS
                saio.sa_restorer = NULL;
        #endif

	saio.sa_handler = SIG_IGN;	// SIG_IGN to
	sigaction(SIGHUP,&saio,NULL);		// IGNORE SIGHUP
	return 0;
}


// copied from signal_handling.c

const char* SIGNAME[] = {"null", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP",
			"SIGABRT", "SIGBUS	", "SIGFPE", "SIGKILL", "SIGUSR1",
			"SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM",
			"SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP",
			"SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ",
			"SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGIO", "SIGPWR",
			"SIGSYS", "SIGRTMIN", "SIGRTMIN+1", "SIGRTMIN+2", "SIGRTMIN+3",
			"SIGRTMIN+4", "SIGRTMIN+5", "SIGRTMIN+6", "SIGRTMIN+7",
			"SIGRTMIN+8", "SIGRTMIN+9", "SIGRTMIN+10", "SIGRTMIN+11",
			"SIGRTMIN+12", "SIGRTMIN+13", "SIGRTMIN+14", "SIGRTMAX-14",
			"SIGRTMAX-13", "SIGRTMAX-12", "SIGRTMAX-11", "SIGRTMAX-10",
			"SIGRTMAX-9", "SIGRTMAX-8", "SIGRTMAX-7", "SIGRTMAX-6",
			"SIGRTMAX-5", "SIGRTMAX-4", "SIGRTMAX-3", "SIGRTMAX-2",
			"SIGRTMAX-1", "SIGRTMAX" };

int signal_handler(int sig, void (*func)(int)) {
	struct sigaction sa;
	sa.sa_handler = func;
	sigemptyset(&(sa.sa_mask)); // see man sigsetops(3)
	sa.sa_flags = 0;
	return sigaction(sig, &sa, NULL);	// ACTION
}

int signal_block(int sig) {
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, sig);
	return sigprocmask(SIG_BLOCK, &sigset, NULL); // BLOCK
}

int signal_unblock(int sig) {
	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, sig);
	return sigprocmask(SIG_UNBLOCK, &sigset, NULL);	// UNBLOCK
}

