#include <syscall.h>
#include <stdint.h>
#include <kernel.h>
#include <unistd.h>

static const long IGNORE = 0L;

static long invoke_syscall(long sysno, long arg0, long arg1, long arg2,
                           long arg3, long arg4)
{
    /* TODO: [p2-task3] implement invoke_syscall via inline assembly */
    asm volatile("nop");

    return 0;
}

void sys_yield(void)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_yield */
    /* TODO: [p2-task3] call invoke_syscall to implement sys_yield */
}

void sys_move_cursor(int x, int y)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_move_cursor */
    /* TODO: [p2-task3] call invoke_syscall to implement sys_move_cursor */
}

void sys_write(char *buff)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_write */
    /* TODO: [p2-task3] call invoke_syscall to implement sys_write */
}

void sys_reflush(void)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_reflush */
    /* TODO: [p2-task3] call invoke_syscall to implement sys_reflush */
}

int sys_mutex_init(int key)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_init */
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_init */
    return 0;
}

void sys_mutex_acquire(int mutex_idx)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_acquire */
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_acquire */
}

void sys_mutex_release(int mutex_idx)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_release */
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_release */
}

long sys_get_timebase(void)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_get_timebase */
    return 0;
}

long sys_get_tick(void)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_get_tick */
    return 0;
}

void sys_sleep(uint32_t time)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_sleep */
}

/************************************************************/
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/