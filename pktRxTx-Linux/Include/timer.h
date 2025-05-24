#ifndef __TIMER_H__
#define __TIMER_H__

#ifndef WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif  // !WIN32

#define DEFAULT_TIMEOUT     25000000  // 25s
#define DEFAULT_INTERVAL     500000  //  5s

typedef struct {
    int enable;             // Whether the timer is enabled
    int timeout;            // If timeout is no more than 0, timer thread will kill receiver thread
#ifndef WIN32
    pthread_t pid;  		// To start a timer thread
    pthread_mutex_t mutex;  // To synchronize the access to member timeout
#else
    HANDLE pid;             // To start a timer thread
    HANDLE mutex;			// To synchronize the access to member timeout
#endif  // !WIN32
} my_timer_t;

extern my_timer_t g_timer;

void set_timer(void);
void unset_timer(void);

#ifndef WIN32
void *timer_thread(void *param);
#else
unsigned int WINAPI timer_thread(void *param);
#endif  // !WIN32

#endif  // !__TIMER_H__
