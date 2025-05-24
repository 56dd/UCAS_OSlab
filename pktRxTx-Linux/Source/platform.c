#include <stdio.h>
#include <stdlib.h>

#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#else
#include <process.h>
#include <windows.h>
#include <shlobj.h>
#endif  // !WIN32

#include "common.h"

int platform_is_admin(void)
{
#ifndef WIN32
    return (int)(0 == getuid());
#else
    return (int)IsUserAnAdmin();
#endif  // !WIN32
}

void platform_create_thread(void *param, void *func)
{
#ifndef WIN32
    pthread_t *pid = param;
    void *(*start)(void *) = (void *(*)(void *))func;

    if (0 != pthread_create(pid, NULL, start, NULL))
    {
        printf("[%s] Error: Failed to create thread!\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }
#else
    HANDLE *handle_ptr = (HANDLE *)param;
    unsigned int WINAPI(*start)(void *) = \
        (unsigned int WINAPI(*)(void *))func;

    *handle_ptr = (HANDLE)_beginthreadex(NULL, 0, start, NULL, 0, NULL);

    if (NULL == *handle_ptr)
    {
        printf("[%s] Error: Failed to create thread!\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }
#endif  // !WIN32
}

void platform_destroy_thread(void *param)
{
#ifndef WIN32
    pthread_t *pid = (pthread_t *)param;
    pthread_t cur_pid = pthread_self();

    if (*pid == cur_pid)
    {
        pthread_exit(NULL);
    }
    else
    {
        pthread_kill(*pid, SIGKILL);
    }
#else
    HANDLE *handle_ptr = (HANDLE *)param;
    HANDLE cur_handle = GetCurrentThread();

    if (*handle_ptr == cur_handle)
    {
        ExitThread(1);
    }
    else
    {
        TerminateThread(*handle_ptr, 1);
    }
#endif  // !WIN32
}

void platform_wait_thread(void *param)
{
#ifndef WIN32
    pthread_t *pid = (pthread_t *)param;
    pthread_join(*pid, NULL);
#else
    HANDLE *handle_ptr = (HANDLE *)param;
    WaitForSingleObject(*handle_ptr, INFINITE);
#endif  // !WIN32
}

int platform_is_current_thread(void *param)
{
#ifndef WIN32
    pthread_t *pid = (pthread_t *)param;
    return (int)(*pid == pthread_self());
#else
    HANDLE *handle_ptr = (HANDLE *)param;
    return (int)(*handle_ptr == GetCurrentThread());
#endif  // !WIN32
}

void platform_mutex_init(void *param)
{
#ifndef WIN32
    pthread_mutex_t *mutex = (pthread_mutex_t *)param;
    pthread_mutex_init(mutex, NULL);
#else
    HANDLE *handle_ptr = (HANDLE *)param;
    *handle_ptr = CreateMutex(NULL, FALSE, NULL);
#endif  // !WIN32
}

void platform_mutex_destroy(void *param)
{
#ifndef WIN32
    pthread_mutex_t *mutex = (pthread_mutex_t *)param;
    pthread_mutex_destroy(mutex);
#else
    HANDLE *handle_ptr = (HANDLE *)param;
    CloseHandle(*handle_ptr);
#endif  // !WIN32
}

void platform_mutex_lock(void *param)
{
#ifndef WIN32
    pthread_mutex_t *mutex = (pthread_mutex_t *)param;
    pthread_mutex_lock(mutex);
#else
    HANDLE *handle_ptr = (HANDLE *)param;
    WaitForSingleObject(*handle_ptr, INFINITE);
#endif  // !WIN32
}

void platform_mutex_unlock(void *param)
{
#ifndef WIN32
    pthread_mutex_t *mutex = (pthread_mutex_t *)param;
    pthread_mutex_unlock(mutex);
#else
    HANDLE *handle_ptr = (HANDLE *)param;
    ReleaseMutex(*handle_ptr);
#endif  // !WIN32
}

void platform_wait_us(int us)
{
#ifndef WIN32
    struct timeval tv_start;
    struct timeval tv_end;
    long diff_time = 0;
    long threshold = (long)us;

    gettimeofday(&tv_start, NULL);

    while (1)
    {
        gettimeofday(&tv_end, NULL);

        diff_time = (tv_end.tv_sec - tv_start.tv_sec) * 1e6 \
                  + tv_end.tv_usec - tv_start.tv_usec;
        
        if (diff_time >= threshold)
        {
            break;
        }
    }
#else
    LARGE_INTEGER frequency = { 0 };
    if (!QueryPerformanceFrequency(&frequency)) 
    {
        printf("[%s] Error: QueryPerformanceFrequency not support for this machine!\n",\
                __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    LARGE_INTEGER timeStart = { 0 };
    QueryPerformanceCounter(&timeStart);
    LARGE_INTEGER timeEnd = { 0 };

    while (1)
    {
        QueryPerformanceCounter(&timeEnd);
        double time = ((timeEnd.QuadPart - timeStart.QuadPart) * 1e6 \
            / (double)frequency.QuadPart);
        if (time >= us)
        {
            break;
        }
    }
#endif  // !WIN32
}