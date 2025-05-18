#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <printk.h>

spin_lock_t klock;  // 大内核锁
uint64_t cpu_id;
void smp_init()
{
    /* TODO: P3-TASK3 multicore*/
    spin_lock_init(&klock);
}

void wakeup_other_hart()
{
    /* TODO: P3-TASK3 multicore*/
    send_ipi(NULL);
}

void lock_kernel()
{
    /* TODO: P3-TASK3 multicore*/
    spin_lock_acquire(&klock);
}

void unlock_kernel()
{
    /* TODO: P3-TASK3 multicore*/
    spin_lock_release(&klock);
}
