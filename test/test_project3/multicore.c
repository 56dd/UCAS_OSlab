#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define MAX_RANGE 5000000
#define MOD 1000007
#define NUM_CPUS 2
#define BUF_LEN 30
#define TEST_SINGLE_CORE (0x56001000)
#define TEST_MULTI_CORE (0x56002000)

struct TestMultiCoreArg
{
    int print_location;
    int from;
    int to;
    int* result;
};

int main(int argc, char* argv[])
{
#ifndef S_CORE
    assert(argc >= 1);
    int print_location = (argc == 1) ? 0 : atoi(argv[1]);
    sys_move_cursor(0, print_location);

    printf("start test multi-core performance\n\r");
    int single_core_result = 0;
    struct TestMultiCoreArg *singleCoreArg = (struct TestMultiCoreArg *)TEST_SINGLE_CORE;
    // *singleCoreArg = {0, 0, MAX_RANGE, &single_core_result};
    singleCoreArg->print_location = print_location;
    singleCoreArg->from = 0;
    singleCoreArg->to = MAX_RANGE;
    singleCoreArg->result = &single_core_result;
    
    // single core performance
    clock_t singleCoreBegin = clock();

    char singlecore_buf[BUF_LEN];
    assert(itoa((int)singleCoreArg, singlecore_buf, BUF_LEN, 10) != -1);
    char *argv_singlecore[2] = {"add", singlecore_buf};

    pid_t single_pid = sys_exec(argv_singlecore[0], 2, argv_singlecore);

    sys_waitpid(single_pid);
    clock_t singleCoreEnd = clock();
    sys_move_cursor(0, print_location + 5);
    printf("single core: %ld ticks, result = %d            \n\r", singleCoreEnd - singleCoreBegin, single_core_result);

    struct TestMultiCoreArg *multiCoreArgs = (struct TestMultiCoreArg *)TEST_MULTI_CORE;
    pid_t pids[NUM_CPUS];
    int multi_core_results[NUM_CPUS] = {0};
    for (int i = 0; i < NUM_CPUS; ++i) {
        multiCoreArgs[i].print_location = i + print_location;
        multiCoreArgs[i].from = MAX_RANGE * i / NUM_CPUS;
        multiCoreArgs[i].to = MAX_RANGE * (i + 1) / NUM_CPUS;
        multiCoreArgs[i].result = &multi_core_results[i];
    }

    clock_t multiCoreBegin = clock();
    for (int i = 0; i < NUM_CPUS; ++i) {
        char multicore_buf[BUF_LEN];
        assert(itoa((int)&multiCoreArgs[i], multicore_buf, BUF_LEN, 10) != -1);
        char *argv_multicore[2] = {"add", multicore_buf};
        pids[i] = sys_exec(argv_multicore[0], 2, argv_multicore);
    }

    for (int i = 0; i < NUM_CPUS; ++i) {
        sys_waitpid(pids[i]);
    }
    int multi_core_final_result = 0;
    for (int i = 0; i < NUM_CPUS; ++i) {
        multi_core_final_result += multi_core_results[i];
	multi_core_final_result = multi_core_final_result % MOD;
    }
    clock_t multiCoreEnd = clock();
    sys_move_cursor(0, print_location + 6);
    printf("multi core: %ld ticks, result = %d             \n\r", multiCoreEnd - multiCoreBegin, multi_core_final_result);
#endif

    sys_exit();    
}

