#include <kernel.h>
#define PIPE_LOC      0x54000000    /* address of pipe      */
char buf[20];
int main()
{
    int number;
    int *ptr = (int *)PIPE_LOC;
    number = *ptr;
    number = number * number;
    *ptr = number;
    my_itoa(number, 10, 0, 0, buf, 0);
    bios_putstr("square=");
    bios_putstr(buf);
    bios_putchar('\n');
    return 0;
}