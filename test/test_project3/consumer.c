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
    int consumption = 1;
    int sum_consumption = 0;
    int consumption_in_second = 0;
    int consumption_in_10second = 0;
    int consumption_before = 0;
    int consumption_before10 = 0;
    // Initialize condition
    int handle_cond = sys_condition_init(COND_KEY);
    int handle_lock = sys_mutex_init(LOCK_KEY);
    int * num_staff = (int*)(0x56000000);
    uint32_t time_base = sys_get_timebase();
    int i= clock()/time_base;
    int j= clock()/time_base;

    while (1)
    {
        sys_mutex_acquire(handle_lock);

        while (*(num_staff) == 0)
        {
            sys_condition_wait(handle_cond, handle_lock);
        }

        *(num_staff) -= consumption;
        sum_consumption += consumption;

        uint32_t time_elapsed = clock();
        uint32_t time = time_elapsed / time_base;
        //if(time>i+1){
        //    consumption_in_second = sum_consumption - consumption_before;
        //    sys_move_cursor(0, 0);
        //    printf("In time [%d], we consume [%d]\n",time,consumption_in_second);
        //    i++;
        //    consumption_before = sum_consumption;
        //}
        if(time>j+10){
            consumption_in_10second = sum_consumption - consumption_before10;
            sys_move_cursor(0, 0);
            printf("last 10s, i consume %d per s\n",consumption_in_10second/10);
            j+=10;
            consumption_before10 = sum_consumption;
        }

        sys_mutex_release(handle_lock);
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
#endif

    return 0;
}