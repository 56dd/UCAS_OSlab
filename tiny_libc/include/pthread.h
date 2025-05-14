#ifndef PTHREAD_H_
#define PTHREAD_H_
#include "unistd.h"

/* TODO:[P4-task4] pthread_create/wait */
void pthread_create(pthread_t *thread,
                   void (*start_routine)(void*),
                   void *arg);

int pthread_join(pthread_t thread);


#endif