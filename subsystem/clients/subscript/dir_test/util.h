#ifndef UTIL_H
#define UTIL_H

#include <signal.h>
int register_handler(int sig, void (*func)(int));

void stat_print(char* filename);

#endif //ndef UTIL_H
