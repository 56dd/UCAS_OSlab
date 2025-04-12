#include <atomic.h>
#include <os/sched.h>
#include <os/smp.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <printk.h>

void smp_init()
{
    /* TODO: P3-TASK3 multicore*/
    spin_lock_init(&klock);
    spin_lock_init(&bios_lock);  
    spin_lock_init(&screen_lock);
    spin_lock_init(&sched_lock);

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

void unlock_sched()
{
    spin_lock_release(&sched_lock);
}

