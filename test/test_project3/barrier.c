#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define NUM_TB 3
#define BARR_KEY 58
#define BUF_LEN 20

#ifdef S_CORE
int main(int argc, int arg0)
{
    int print_location = (argc == 1) ? 0 : arg0;

    // Initialize barrier
    int handle = sys_barrier_init(BARR_KEY, NUM_TB);

    // Launch child processes
    pid_t pids[NUM_TB];
    
    // Start three test programs
    for (int i = 0; i < NUM_TB; ++i)
    {
        // TODO: [P3-TASK2 S-core] use your "test_barrier" id here        
        int tb_id = -1;
        assert(tb_id != -1);
        pids[i] = sys_exec(tb_id, 2, print_location + i, handle, 0);       
    }

    // Wait child processes to exit
    for (int i = 0; i < NUM_TB; ++i)
    {
        sys_waitpid(pids[i]);
    }

    // Destroy barrier
    sys_barrier_destroy(handle);

    return 0;
}

#else
int main(int argc, char *argv[])
{
    assert(argc >= 1);
    int print_location = (argc == 1) ? 0 : atoi(argv[1]);

    // Initialize barrier
    int handle = sys_barrier_init(BARR_KEY, NUM_TB);

    // Launch child processes
    pid_t pids[NUM_TB];
    
    // Start three test programs
    for (int i = 0; i < NUM_TB; ++i)
    {
        char buf_location[BUF_LEN];
        char buf_handle[BUF_LEN];
        assert(itoa(print_location + i, buf_location, BUF_LEN, 10) != -1);
        assert(itoa(handle, buf_handle, BUF_LEN, 10) != -1);

        char *argv[3] = {"test_barrier", buf_location, buf_handle};
        pids[i] = sys_exec(argv[0], 3, argv);

    }

    // Wait child processes to exit
    for (int i = 0; i < NUM_TB; ++i)
    {
        sys_waitpid(pids[i]);
    }

    // Destroy barrier
    sys_barrier_destroy(handle);

    return 0;
}
#endif