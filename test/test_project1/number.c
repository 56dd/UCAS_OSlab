#include <kernel.h>
#define PIPE_LOC      0x54000000    /* address of pipe      */

char buf[10];
int main()
{
    int number;
    int tmp;
    int i=0;
    bios_putstr("请输入数字:");
    while(1){
    while((tmp = bios_getchar())==-1);
        if(tmp == '\r')
        {
            bios_putchar('\n');
            buf[i++]='\0';
            break;
        }else{
            bios_putchar(tmp);
            buf[i++]=tmp;
        }
    }
    tmp = my_atoi(buf);
    number = tmp;
    bios_putstr("数字为:");
    bios_putstr(buf);
    bios_putchar('\n');
    int *ptr = (int *)PIPE_LOC;
    *ptr = number;
    return 0;
}