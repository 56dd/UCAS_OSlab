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

    int consumption = 1;
    int sum_consumption = 0;

    while (1)
    {
        sys_mutex_acquire(handle_lock);

        while (*(num_staff) == 0)
        {
            sys_condition_wait(handle_cond, handle_lock);
        }

        *(num_staff) -= consumption;
        sum_consumption += consumption;

        int next;
        while((next = rand() % 3) == 0);
        sys_move_cursor(0, print_location);
        printf("> [TASK] Total consumed %d products. (Sleep %d seconds)", sum_consumption, next);

        sys_mutex_release(handle_lock);
        sys_sleep(next);
    }
#endif

    return 0;
}