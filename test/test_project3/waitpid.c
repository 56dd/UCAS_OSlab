#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define LOCK1_KEY 20
#define LOCK2_KEY 22

#define BUF_LEN 10

#ifdef S_CORE
int main(int argc, int arg0)
{
    int print_location = (argc == 1) ? 0 : arg0;

    // Init two mutex locks, and convert decimal handle to string via itoa
    int handle1 = sys_mutex_init(LOCK1_KEY);
    int handle2 = sys_mutex_init(LOCK2_KEY);


    // Launch two assistant processes, and pass mutex handles to them
    // TODO: [P3-TASK1 S-core] use your "ready_to_exit" id here
    int rte_id = -1;
    assert(rte_id != -1);
    pid_t pid1 = sys_exec(rte_id, 3, print_location + 1, handle1, handle2);

    sys_sleep(1);  // wait enough time for task1 to acquire locks
    

    // TODO: [P3-TASK1 S-core] use your "wait_locks" id here
    int wl_id = -1;
    assert(wl_id != -1);
    pid_t pid2 = sys_exec(wl_id, 3, print_location + 2, handle1, handle2);

    // Start to waitpid(pid1)
    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=%d) to exit.", pid1);
    sys_waitpid(pid1);

    // Finish waitpid(pid1)
    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", pid1);

    // Start to waitpid(pid2)
    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=%d) to exit.", pid2);
    sys_waitpid(pid2);

    // Finish waitpid(pid2)
    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", pid2);

    return 0;
}

#else
int main(int argc, char *argv[])
{
    assert(argc >= 1);
    int print_location = (argc == 1) ? 0 : atoi(argv[1]);


    // Init two mutex locks, and convert decimal handle to string via itoa
    int handle1 = sys_mutex_init(LOCK1_KEY);
    int handle2 = sys_mutex_init(LOCK2_KEY);

    char buf1[BUF_LEN];
    char buf2[BUF_LEN];
    assert(itoa(handle1, buf1, BUF_LEN, 10) != -1);
    assert(itoa(handle2, buf2, BUF_LEN, 10) != -1);

    // Launch two assistant processes, and pass mutex handles to them
    char location1[BUF_LEN];
    assert(itoa(print_location + 1, location1, BUF_LEN, 10) != -1);

    char *argv1[4] = {"ready_to_exit", location1, buf1, buf2};
    pid_t pid1 = sys_exec(argv1[0], 4, argv1);

    sys_sleep(1);  // wait enough time for task1 to acquire locks
    
    char location2[BUF_LEN];
    assert(itoa(print_location + 2, location2, BUF_LEN, 10) != -1);

    char *argv2[4] = {"wait_locks", location2, buf1, buf2};
    pid_t pid2 = sys_exec(argv2[0], 4, argv2);

    // Start to waitpid(pid1)
    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=%d) to exit.", pid1);
    sys_waitpid(pid1);

    // Finish waitpid(pid1)
    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", pid1);

    // Start to waitpid(pid2)
    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=%d) to exit.", pid2);
    sys_waitpid(pid2);

    // Finish waitpid(pid2)
    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", pid2);

    return 0;
}
#endif