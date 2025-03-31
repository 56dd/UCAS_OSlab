#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define NUM_TC 3
#define COND_KEY 58
#define LOCK_KEY 42
#define RESOURCE_ADDR 0x56000000
#define BUF_LEN 30


int main(int argc, char * argv[])
{
#ifndef S_CORE
    assert(argc >= 1);
    int print_location = (argc == 1) ? 0 : atoi(argv[1]);

    // Initialize condition
    int handle_cond = sys_condition_init(COND_KEY);
    int handle_lock = sys_mutex_init(LOCK_KEY);

    // Initialize num_staff zero
    *(int *)RESOURCE_ADDR = 0;

    // Launch child processes
    pid_t pids[NUM_TC + 1];

    char buf_location[BUF_LEN];
    char buf_cond_handle[BUF_LEN];
    char buf_lock_handle[BUF_LEN];
    char buf_resource[BUF_LEN];    

    assert(itoa(print_location + 0, buf_location, BUF_LEN, 10) != -1);
    assert(itoa(handle_cond, buf_cond_handle, BUF_LEN, 10) != -1);
    assert(itoa(handle_lock, buf_lock_handle, BUF_LEN, 10) != -1);
    assert(itoa(RESOURCE_ADDR, buf_resource, BUF_LEN, 10) != -1);

    // Start producer
    char *argv_producer[5] = {"producer", \
                              buf_location, \
                              buf_cond_handle, \
                              buf_lock_handle, \
                              buf_resource \
                              };
    pids[0] = sys_exec(argv_producer[0], 5, argv_producer);

    // Start consumers
    for (int i = 1; i < NUM_TC + 1; i++)
    {
        assert(itoa(print_location + i, buf_location, BUF_LEN, 10) != -1);

        char *argv_consumer[5] = {"consumer", \
                                  buf_location, \
                                  buf_cond_handle, \
                                  buf_lock_handle, \
                                  buf_resource \
                                  };
        pids[i] = sys_exec(argv_consumer[0], 5, argv_consumer);
    }

    // Wait produce processes to exit
    for (int i = 0; i < NUM_TC + 1; i++) {
        sys_waitpid(pids[i]);
    }
    
    // Destroy condition
    sys_condition_destroy(handle_cond);
#endif

    return 0;    
}