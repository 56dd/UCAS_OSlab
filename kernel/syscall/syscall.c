#include <sys/syscall.h>
#include <asm/unistd.h>
#include <os/smp.h>
#include <os/lock.h>
#include <printk.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    /* TODO: [p2-task3] handle syscall exception */
    /**
     * HINT: call syscall function like syscall[fn](arg0, arg1, arg2),
     * and pay attention to the return value and sepc
     */
    regs->sepc += 4; /* when return from syscall, skip the "ecall" */

    switch (regs->regs[17]) {
    case SYSCALL_EXEC:
    case SYSCALL_EXIT:
    case SYSCALL_SLEEP:
    case SYSCALL_KILL:
    case SYSCALL_WAITPID:
    case SYSCALL_GETPID:
    case SYSCALL_YIELD:
    case SYSCALL_GET_TIMEBASE:
    case SYSCALL_GET_TICK:
    case SYSCALL_SET_SCHE_WORKLOAD:
    case SYSCALL_LOCK_INIT:
    case SYSCALL_LOCK_ACQ:
    case SYSCALL_LOCK_RELEASE:
    case SYSCALL_SHOW_TASK:
    case SYSCALL_BARR_INIT:
    case SYSCALL_BARR_WAIT:
    case SYSCALL_BARR_DESTROY:
    case SYSCALL_COND_INIT:
    case SYSCALL_COND_WAIT:
    case SYSCALL_COND_SIGNAL:
    case SYSCALL_COND_BROADCAST:
    case SYSCALL_COND_DESTROY:
    case SYSCALL_MBOX_OPEN:
    case SYSCALL_MBOX_CLOSE:
    case SYSCALL_MBOX_SEND:
    case SYSCALL_MBOX_RECV:
    case SYSCALL_TASKSET:
         spin_lock_acquire(&sched_lock);
         sched_cpu_id = get_current_cpu_id();
         printl("I'am Pid[%d],i'am on Core[%d], Now i take the sched_lock.\n",sched_cpu_id,current_running[sched_cpu_id]->pid);
         break;
    case SYSCALL_WRITE:
    case SYSCALL_CURSOR:
    case SYSCALL_REFLUSH:
    case SYSCALL_CLEAR:
    case SYSCALL_WRITECH:
         spin_lock_acquire(&screen_lock);
         screen_cpu_id = get_current_cpu_id();
         printl("I'am Pid[%d],i'am on Core[%d],Now i take the screen_lock.\n",screen_cpu_id,current_running[screen_cpu_id]->pid);
         break;
    case SYSCALL_READCH:
         spin_lock_acquire(&bios_lock);
         bios_cpu_id = get_current_cpu_id();
         //printl("I'am Pid[%d],Now i take the bios_lock.\n",current_running[bios_cpu_id]->pid);
         break; 
    case SYSCALL_PS:
         spin_lock_acquire(&sched_lock);
         spin_lock_acquire(&screen_lock);
         screen_cpu_id = get_current_cpu_id();
         printl("I'am Pid[%d],i'am on Core[%d],Now i take the screen_lock and sched_lock.\n",screen_cpu_id,current_running[screen_cpu_id]->pid);
         break;
    }

    regs->regs[10] = syscall[regs->regs[17]](   /* x17: a7 */
        regs->regs[10],                         /* x10: a0 */
        regs->regs[11],                         /* x11: a1 */
        regs->regs[12],                         /* x12: a2 */
        regs->regs[13],                         /* x13: a3 */
        regs->regs[14]                          /* x14: a4 */
    );

    switch (regs->regs[17]) {
    case SYSCALL_EXEC:
    case SYSCALL_EXIT:
    case SYSCALL_SLEEP:
    case SYSCALL_KILL:
    case SYSCALL_WAITPID:
    case SYSCALL_GETPID:
    case SYSCALL_YIELD:
    case SYSCALL_GET_TIMEBASE:
    case SYSCALL_GET_TICK:
    case SYSCALL_SET_SCHE_WORKLOAD:
    case SYSCALL_LOCK_INIT:
    case SYSCALL_LOCK_ACQ:
    case SYSCALL_LOCK_RELEASE:
    case SYSCALL_SHOW_TASK:
    case SYSCALL_BARR_INIT:
    case SYSCALL_BARR_WAIT:
    case SYSCALL_BARR_DESTROY:
    case SYSCALL_COND_INIT:
    case SYSCALL_COND_WAIT:
    case SYSCALL_COND_SIGNAL:
    case SYSCALL_COND_BROADCAST:
    case SYSCALL_COND_DESTROY:
    case SYSCALL_MBOX_OPEN:
    case SYSCALL_MBOX_CLOSE:
    case SYSCALL_MBOX_SEND:
    case SYSCALL_MBOX_RECV:
    case SYSCALL_TASKSET:
         printl("I'am Pid[%d],i'am on Core[%d], Now i will release the sched_lock.\n",sched_cpu_id,current_running[sched_cpu_id]->pid);
         spin_lock_release(&sched_lock);
         break;
    case SYSCALL_WRITE:
    case SYSCALL_CURSOR:
    case SYSCALL_REFLUSH:
    case SYSCALL_CLEAR:
    case SYSCALL_WRITECH:
         printl("I'am Pid[%d],i am on Core[%d], Now i will release the screen_lock.\n",screen_cpu_id,current_running[screen_cpu_id]->pid);
         spin_lock_release(&screen_lock);
         break;
    case SYSCALL_READCH:
         spin_lock_release(&bios_lock);
         //printl("I'am Pid[%d],Now i will release the bios_lock.\n",current_running[bios_cpu_id]->pid);
         break; 
    case SYSCALL_PS:
         printl("I'am Pid[%d],i am on Core[%d], Now i will release the screen_lock and sched_lock.\n",screen_cpu_id,current_running[screen_cpu_id]->pid);
         spin_lock_release(&screen_lock);
         spin_lock_release(&sched_lock);
         break;
    }

}
