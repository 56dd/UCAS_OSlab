#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define NUM_TC 3
#define SEMA_KEY 33
#define BUF_LEN 30


int main(int argc, char * argv[])
{
    assert(argc >= 1);
    int print_location = (argc == 1) ? 0 : atoi(argv[1]);

    // Initialize semaphore
    int handle_sema = sys_semaphore_init(SEMA_KEY, 15);

    // Launch child processes
    pid_t pids[NUM_TC<<1];

    char buf_location[BUF_LEN];
    char buf_sem_handle[BUF_LEN];    

    assert(itoa(handle_sema, buf_sem_handle, BUF_LEN, 10) != -1);

    // Start producer
    for (int i = 0; i < NUM_TC; i++){
        assert(itoa(print_location + i, buf_location, BUF_LEN, 10) != -1);
        char *argv_producer[3] = {"sema_producer", \
                                buf_location, \
                                buf_sem_handle\
                                };
        pids[i] = sys_exec(argv_producer[0], 3, argv_producer);
    }

    // Start consumers
    for (int i = NUM_TC; i < NUM_TC<<1; i++)
    {
        assert(itoa(print_location + i, buf_location, BUF_LEN, 10) != -1);

        char *argv_consumer[3] = {"sema_consumer", \
                                  buf_location, \
                                  buf_sem_handle\
                                  };
        pids[i] = sys_exec(argv_consumer[0], 3, argv_consumer);
    }

    // Wait produce processes to exit
    for (int i = 0; i < NUM_TC<<1; i++) {
        sys_waitpid(pids[i]);
    }
    
    // Destroy semaphore
    sys_semaphore_destroy(handle_sema);

    return 0;    
}