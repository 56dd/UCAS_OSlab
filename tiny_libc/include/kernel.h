#ifndef __INCLUDE_KERNEL_H__
#define __INCLUDE_KERNEL_H__

#include <stdint.h>

#define KERNEL_JMPTAB_BASE 0x51ffff00
typedef enum {
    CONSOLE_PUTSTR,
    CONSOLE_PUTCHAR,
    CONSOLE_GETCHAR,
    SD_READ,
    SD_WRITE,
    QEMU_LOGGING,
    SET_TIMER,
    READ_FDT,
    MOVE_CURSOR,
    WRITE,
    REFLUSH,
    PRINT,
    YIELD,
    MUTEX_INIT,
    MUTEX_ACQ,
    MUTEX_RELEASE,
    NUM_ENTRIES
} jmptab_idx_t;


static inline long call_jmptab(long which, long arg0, long arg1, long arg2, long arg3, long arg4)
{
    unsigned long val = \
        *(unsigned long *)(KERNEL_JMPTAB_BASE + sizeof(unsigned long) * which);
    long (*func)(long, long, long, long, long) = (long (*)(long, long, long, long, long))val;

    return func(arg0, arg1, arg2, arg3, arg4);
}

static inline void bios_putstr(char *str)
{
    call_jmptab(CONSOLE_PUTSTR, (long)str, 0, 0, 0, 0);
}

static inline void bios_putchar(int ch)
{
    call_jmptab(CONSOLE_PUTCHAR, (long)ch, 0, 0, 0, 0);
}

static inline int bios_getchar(void)
{
    return call_jmptab(CONSOLE_GETCHAR, 0, 0, 0, 0, 0);
}

unsigned int my_itoa(
    long value, unsigned int radix, unsigned int uppercase,
    unsigned int unsig, char *buffer, unsigned int zero_pad)
{
    char *pbuffer = buffer;
    int negative  = 0;
    unsigned int i, len;

    /* No support for unusual radixes. */
    if (radix > 16) return 0;

    if (value < 0 && !unsig) {
        negative = 1;
        value    = -value;
    }

    /* This builds the string back to front ... */
    do {
        int digit = value % radix;
        *(pbuffer++) =
            (digit < 10 ? '0' + digit :
                          (uppercase ? 'A' : 'a') + digit - 10);
        value /= radix;
    } while (value > 0);

    for (i = (pbuffer - buffer); i < zero_pad; i++)
        *(pbuffer++) = '0';

    if (negative) *(pbuffer++) = '-';

    *(pbuffer) = '\0';

    /* ... now we reverse it (could do it recursively but will
     * conserve the stack space) */
    len = (pbuffer - buffer);
    for (i = 0; i < len / 2; i++) {
        char j              = buffer[i];
        buffer[i]           = buffer[len - i - 1];
        buffer[len - i - 1] = j;
    }

    return len;
}


int my_atoi(const char *str) {
    int result = 0;
    int sign = 1;
    // 跳过前导空格
    while (*str == ' ') {
        str++;
    }
    // 处理正负号
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }
    // 转换数字字符为整数
    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';
        result = result * 10 + digit;
        str++;
    }
    // 应用符号
    return sign * result;
}


#endif