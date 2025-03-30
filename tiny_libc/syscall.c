#include <syscall.h>
#include <stdint.h>
#include <kernel.h>
#include <unistd.h>

static const long IGNORE = 0L;

static long invoke_syscall(long sysno, long arg0, long arg1, long arg2,
                           long arg3, long arg4)
{
    /* TODO: [p2-task3] implement invoke_syscall via inline assembly */
    long res;
    asm volatile(
        "mv     a7, %1\n\t"     /* a7: sysno       */
        "mv     a0, %2\n\t"     /* a0: arg0        */
        "mv     a1, %3\n\t"     /* a1: arg1        */
        "mv     a2, %4\n\t"     /* a2: arg2        */
        "mv     a3, %5\n\t"     /* a3: arg3        */
        "mv     a4, %6\n\t"     /* a4: arg4        */
        "ecall        \n\t"     /* syscall         */
        "mv     %0, a0\n\t"     /* a0:return value */
        :"=r"(res)
        :"r"(sysno), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
    );

    return res;
}

void sys_yield(void)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_yield */
    //call_jmptab(YIELD, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
    /* TODO: [p2-task3] call invoke_syscall to implement sys_yield */
    invoke_syscall(SYSCALL_YIELD, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_move_cursor */
    //call_jmptab(MOVE_CURSOR, x, y, IGNORE, IGNORE, IGNORE);
    /* TODO: [p2-task3] call invoke_syscall to implement sys_move_cursor */
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE, IGNORE, IGNORE);
}


void sys_write(char *buff)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_write */
    //call_jmptab(WRITE,(long)buff,IGNORE,IGNORE,IGNORE,IGNORE);
    /* TODO: [p2-task3] call invoke_syscall to implement sys_write */
    invoke_syscall(SYSCALL_WRITE, (long)buff, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_reflush(void)
{
    /* TODO: [p2-task1] call call_jmptab to implement sys_reflush */
    //call_jmptab(REFLUSH,IGNORE,IGNORE,IGNORE,IGNORE,IGNORE);
    /* TODO: [p2-task3] call invoke_syscall to implement sys_reflush */
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

int sys_mutex_init(int key)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_init */
    //call_jmptab(MUTEX_INIT, (long)key,IGNORE,IGNORE,IGNORE,IGNORE);
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_init */
    return invoke_syscall(SYSCALL_LOCK_INIT, (long)key,IGNORE,IGNORE,IGNORE,IGNORE);
}

void sys_mutex_acquire(int mutex_idx)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_acquire */
    //call_jmptab(MUTEX_ACQ, (long)mutex_idx,IGNORE,IGNORE,IGNORE,IGNORE);
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_acquire */
    invoke_syscall(SYSCALL_LOCK_ACQ, (long)mutex_idx,IGNORE,IGNORE,IGNORE,IGNORE);
}

void sys_mutex_release(int mutex_idx)
{
    /* TODO: [p2-task2] call call_jmptab to implement sys_mutex_release */
    //call_jmptab(MUTEX_RELEASE, (long)mutex_idx, IGNORE,IGNORE,IGNORE,IGNORE);
    /* TODO: [p2-task3] call invoke_syscall to implement sys_mutex_release */
    invoke_syscall(SYSCALL_LOCK_RELEASE, (long)mutex_idx,IGNORE,IGNORE,IGNORE,IGNORE);
}

long sys_get_timebase(void)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_get_timebase */
    return invoke_syscall(SYSCALL_GET_TIMEBASE, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

long sys_get_tick(void)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_get_tick */
    return invoke_syscall(SYSCALL_GET_TICK, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_sleep(uint32_t time)
{
    /* TODO: [p2-task3] call invoke_syscall to implement sys_sleep */
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE, IGNORE, IGNORE);
}

void sys_set_sche_workload(int position)
{
    invoke_syscall(SYSCALL_SET_SCHE_WORKLOAD, position, IGNORE, IGNORE, IGNORE, IGNORE);
}

/************************************************************/
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/