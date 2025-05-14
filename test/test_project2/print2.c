#include <stdio.h>
#include <unistd.h>

/**
 * NOTE: bios APIs is used for p2-task1 and p2-task2. You need to change
 * to syscall APIs after implementing syscall in p2-task3!
*/
int main(void)
{
    int print_location = 1;

    for (int i = 0;; i++)
    {
        sys_move_cursor(0, print_location);
        printf("> [TASK] This task is to test scheduler. (%d)", i);
        //sys_yield();
    }
}
