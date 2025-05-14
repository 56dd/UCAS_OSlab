#include <kernel.h>

#define BUF_LEN 50

char buf[BUF_LEN];

int main(void)
{
    // do bss_check
    int check_ok = 1;
    for (int i = 0; i < BUF_LEN; ++i) {
        if (buf[i] != 0) {
            check_ok = 0;
            break;
        }
    }

    if (check_ok) {
        bios_putstr("[bss] Info: passed bss check!\n");
    } else {
        bios_putstr("[bss] Error: bss check failed!\n");
        while (1) ;
    }

    return 0;
}