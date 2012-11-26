/**
 * \file semaphore.h
 * \author Dennis Terhorst
 * \date Mon Sep 21 18:45:49 CEST 2009
 */
#ifndef SEMAPHORE_H
#define SEMAPHORE_H

// there are two kinds of semaphore implementations
// posix is the newer one, but SystemVr4 
#define USE_POSIX_SEMAPHORES

#ifdef USE_POSIX_SEMAPHORES

// POSIX.1-2001 version
#include <semaphore.h>

#else
// SVr4
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

typedef int sem_t;

sem_t *sem_open(const char *name, int oflag);
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
int sem_close(sem_t *sem);
int sem_getvalue(sem_t *sem, int *sval);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
int sem_post(sem_t *sem);


#endif // SVr4

#endif //ndef SEMAPHORE_H
