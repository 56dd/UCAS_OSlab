#include <pthread.h>

/* TODO:[P4-task4] pthread_create/wait */
void pthread_create(pthread_t *thread,
                   void (*start_routine)(void*),
                   void *arg)
{
    /* TODO: [p4-task4] implement pthread_create */
    sys_pthread_create(thread, start_routine, arg);
}

int pthread_join(pthread_t thread)
{
    /* TODO: [p4-task4] implement pthread_join */
    return sys_waitpid(thread);
}

