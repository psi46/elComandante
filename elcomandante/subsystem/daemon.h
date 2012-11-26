/**
 * \file daemon.h
 * \author Dennis Terhorst
 * \date 05/2006
 *
 * \brief Little helpers for daemon processes
 */
#ifndef DAEMON_H
#define DAEMON_H


/** \brief umask of daemon
 *
 * \c daemonize_me() will set the daemons umask to this value.
 */
#define DAEMON_UMASK	0

/** \brief working directory of daemon
 *
 * The daemon process will \c chdir() to this directory. Note that
 * this directory will not be moveable, removeable or unmountable
 * while the daemon process is running.
 */
#define DAEMON_WORKDIR	"/"

// define if daemon should use perror() to report failure
//#define DAEMON_SELFDEBUG

/**
 * \brief convert current process into a daemon
 * 
 * \c daemonize_me forks a new process, the parent exits and
 * the child calls \c setsid(), \c umask() and \c chdir() to
 * become a daemon.
 *
 * \return 0 ok, -1=failed
 */
int daemonize_me();


/**
 * \brief set some adequate signal handler
 * 
 * set signal handlers for \c SIGINT and \c SIGTERM to \a handler
 * set \c SIG_IGN for \c SIG_HUP
 *
 * \param handler function to call on \c SIGINT and \c SIGTERM
 * 
 * \return	always 0 by now
 */
int catch_signals(void (*handler)(int));

/*
 * collection of some other files always used together:
 */

#ifndef SIGNALNAMES_H
#define SIGNALNAMES_H
extern const char* SIGNAME[];
#endif	//ndef SIGNALNAMES_H


#ifndef SIGNAL_HANDLING_H
#define SIGNAL_HANDLING_H
#include <signal.h>
#include <unistd.h> // NULL
int signal_handler(int sig, void (*func)(int));
int signal_block(int sig);
int signal_unblock(int sig);
#endif //ndef SIGNAL_HANDLING_H

#endif //ndef DAEMON_H
