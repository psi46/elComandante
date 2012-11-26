#ifndef UTIL_H
#define UTIL_H

#include <signal.h>
int register_handler(int sig, void (*func)(int));

void fstat_print(int fd);

#endif //ndef UTIL_H
