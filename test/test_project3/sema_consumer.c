#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Error: argc = %d\n", argc);
    }
    assert(argc >= 3);

    int print_location = atoi(argv[1]);
    int handle_sema = atoi(argv[2]);

    int i;
    int sum_consumption = 0;

    printf("> [TASK] (pid=%d) Total consumed 0 products.",sys_getpid());
    for (i = 0; i < 15; i++)
    {
        sys_move_cursor(0, print_location);

        sys_semaphore_down(handle_sema);
        sum_consumption++;
        printf("> [TASK] (pid=%d) Total consumed %d products.",sys_getpid(), sum_consumption);
    }  
    
    return 0;
}