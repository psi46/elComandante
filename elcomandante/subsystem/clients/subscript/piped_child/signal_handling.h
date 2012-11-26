/**
 * \file signal_handling.h
 * \author Dennis Terhorst
 */
#ifndef SIGNAL_HANDLING_H
#define SIGNAL_HANDLING_H

#include <signal.h>
#include <unistd.h> // NULL

#ifndef SIGNALNAMES_H
#define SIGNALNAMES_H
extern const char* SIGNAME[];
#endif	//ndef SIGNALNAMES_H

int signal_handler(int sig, void (*func)(int));
int signal_block(int sig);
int signal_unblock(int sig);

#endif //ndef SIGNAL_HANDLING_H
