#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define COND_KEY 58
#define LOCK_KEY 42
#define C_CORE 0

int main(int argc, char *argv[])
{
#ifdef C_CORE 
    int i;
    int production = 3;
    int sum_production = 0;
    // Initialize condition
    int handle_cond = sys_condition_init(COND_KEY);
    int handle_lock = sys_mutex_init(LOCK_KEY);
    int * num_staff = (int*)(0x56000000);

    for (i = 0;; i++)
    {
        sys_mutex_acquire(handle_lock);

        (*num_staff) += production;
        sum_production += production;

        sys_mutex_release(handle_lock);

        // condition_signal(&condition);
        sys_condition_broadcast(handle_cond);

    }
#else
#ifndef S_CORE
    if (argc < 5)
    {
        printf("Error: argc = %d\n", argc);
    }
    assert(argc >= 5);

    int print_location = atoi(argv[1]);
    int handle_cond = atoi(argv[2]);
    int handle_lock = atoi(argv[3]);
    int * num_staff = (int *)(atoi(argv[4])); 

    // Set random seed
    srand(clock());

    int i;
    int production = 3;
    int sum_production = 0;

    for (i = 0; i < 10; i++)
    {
        sys_mutex_acquire(handle_lock);

        (*num_staff) += production;
        sum_production += production;

        sys_mutex_release(handle_lock);

        sys_move_cursor(0, print_location);
        int next;
        while((next = rand() % 5) == 0);
        printf("> [TASK] Total produced %d products. (next in %d seconds)", sum_production, next);

        // condition_signal(&condition);
        sys_condition_broadcast(handle_cond);

        sys_sleep(next);
    }  
#endif
#endif
    
    return 0;
}