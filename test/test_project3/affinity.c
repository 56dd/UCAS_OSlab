#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define INTEGER_TEST_NUM 5
#define BUF_LEN 20

int main(int argc, char *argv[])
{
#ifndef S_CORE
    assert(argc >= 1);
    int print_location = (argc == 1) ? 0 : atoi(argv[1]);

    srand(42);
    sys_move_cursor(1, 1);
    printf("start test cpu affinity, pids = {");
    int single_core_result = 0;

    pid_t pids[INTEGER_TEST_NUM] = {0};
    for (int i = 0; i < INTEGER_TEST_NUM; ++i) {
        char buf_location[BUF_LEN];
        assert(itoa(print_location + i + 2, buf_location, BUF_LEN, 10) != -1);
        char *argv_affinity[2] = {"test_affinity", buf_location};
        pids[i] = sys_exec(argv_affinity[0], 2, argv_affinity);
        printf("%d, ", pids[i]);
    }
    printf("}\n\r");
    for (int i = 0; i < INTEGER_TEST_NUM; ++i) {
        sys_waitpid(pids[i]);
    }
#endif

    return 0;
}