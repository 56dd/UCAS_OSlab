#include <sys/syscall.h>

long (*syscall[NUM_SYSCALLS])();

void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    /* TODO: [p2-task3] handle syscall exception */
    /**
     * HINT: call syscall function like syscall[fn](arg0, arg1, arg2),
     * and pay attention to the return value and sepc
     */
    regs->sepc += 4; /* when return from syscall, skip the "ecall" */
    regs->regs[10] = syscall[regs->regs[17]](   /* x17: a7 */
        regs->regs[10],                         /* x10: a0 */
        regs->regs[11],                         /* x11: a1 */
        regs->regs[12],                         /* x12: a2 */
        regs->regs[13],                         /* x13: a3 */
        regs->regs[14]                          /* x14: a4 */
    );
}
