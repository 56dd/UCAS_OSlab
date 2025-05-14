#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

int main(int argc, char *argv[])
{
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
    
    return 0;
}