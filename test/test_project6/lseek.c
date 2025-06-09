#include <stdio.h>
#include <string.h>
#include <unistd.h>

static char buff[64];
#define OFFSET  128* 1024 * 1024
 
int main(void)
{
    int fd = sys_open("1.txt", O_RDWR);

    sys_lseek(fd, OFFSET, SEEK_SET);

    // write 'hello world!' * 10
    for (int i = 0; i < 10; i++)
    {
        sys_write(fd, "hello world!\n", 13);
    }

    // read
    for (int i = 0; i < 10; i++)
    {
        sys_read(fd, buff, 13);
        for (int j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    sys_close(fd);

    return 0;
}