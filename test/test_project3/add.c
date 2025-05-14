#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#define MOD 1000007

struct TestMultiCoreArg
{
    int print_location;
    int from;
    int to;
    int* result;
};

int main(int argc, char * argv[])
{
#ifndef S_CORE
    if (argc < 2)
    {
        printf("Error: argc = %d\n", argc);
    }
    assert(argc >= 2);

    
    struct TestMultiCoreArg *args = (struct TestMultiCoreArg *)(atoi(argv[1]));

    int print_location = args->print_location;
    int from  = args->from;
    int to = args->to;
    int result = 0;

    sys_move_cursor(0, print_location);

    printf("start compute, from = %d, to = %d  ", from, to);
    for (int i = from; i < to; ++i) {
        result = (result + i) % MOD;
    }
    printf("Done \n\r");
    *args->result = result;
#endif

    sys_exit();
}