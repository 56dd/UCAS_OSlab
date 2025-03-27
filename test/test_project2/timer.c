#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

int main(void)
{
    int print_location = 5;
    uint32_t time_base = sys_get_timebase();

    while (1)
    {
        uint32_t time_elapsed = clock();
        uint32_t time = time_elapsed / time_base;
        sys_move_cursor(0, print_location);
        printf("> [TASK] This is a thread to timing! (%u/%u seconds).\n",
                time, time_elapsed);
        sys_yield();
    }
}
