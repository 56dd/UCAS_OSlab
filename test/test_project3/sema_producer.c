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

    // Set random seed
    srand(clock());

    int i;
    int sum_production = 0;

    for (i = 0; i < 10; i++)
    {
        sys_move_cursor(0, print_location);
        int next;
        while((next = rand() % 4) == 0);
        sum_production++;
        printf("> [TASK] Total produced %d products. (next in %d seconds)", sum_production, next);
        sys_semaphore_up(handle_sema);

        sys_sleep(next);
    }
    sys_move_cursor(0, print_location);
    printf("> [TASK] Total produced 10 products.                          ");
    
    return 0;
}