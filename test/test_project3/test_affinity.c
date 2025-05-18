#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define MAX_ITERATION 150
#define INTEGER_TEST_CHUNK 100000

char * test[] = 
{
    "1",
    "2",
    "3",
    "5",
    "6",
    "7"
};

int main(int argc, char * argv[])
{
#ifndef S_CORE
    pid_t pid = sys_getpid();
    
    assert(argc >= 1);
    int print_location = (argc == 1) ? 0 : atoi(argv[1]);

    uint64_t ans = 0;
    for (int i = 0; i < MAX_ITERATION; ++i) {
        for (int j = 0; j < INTEGER_TEST_CHUNK; ++j) {
            ans += rand();
	    }
        uint64_t testpc = 0;
        asm volatile("auipc %0, 0x0" : "+r"(testpc));
        sys_move_cursor(1, print_location);
        printf("[%ld] integer test (%d/%d) ", pid, i, MAX_ITERATION);
        printf("access: %s ", test[rand()%6]);
        printf("auipc: %lx\r\n", testpc);
    }
#endif

    return 0;
}
