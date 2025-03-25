#include <asm.h>
#include <csr.h>
#include <asm/regs.h>

.macro SAVE_CONTEXT
  /* TODO: [p2-task3] save all general purpose registers here! */
  /* HINT: Pay attention to the function of tp and sp, and save them carefully! */

  /*
   * Disable user-mode memory access as it should only be set in the
   * actual user copy routines.
   *
   * Disable the FPU to detect illegal usage of floating point in kernel
   * space.
   */
  li t0, SR_SUM | SR_FS

  /* TODO: [p2-task3] save sstatus, sepc, stval and scause on kernel stack */

.endm

.macro RESTORE_CONTEXT
  /* TODO: Restore all general purpose registers and sepc, sstatus */
  /* HINT: Pay attention to sp again! */
.endm

ENTRY(enable_preempt)
  not t0, x0
  csrs CSR_SIE, t0
  jr ra
ENDPROC(enable_preempt)

ENTRY(disable_preempt)
  csrw CSR_SIE, zero
  jr ra
ENDPROC(disable_preempt)

ENTRY(enable_interrupt)
  li t0, SR_SIE
  csrs CSR_SSTATUS, t0
  jr ra
ENDPROC(enable_interrupt)

ENTRY(disable_interrupt)
  li t0, SR_SIE
  csrc CSR_SSTATUS, t0
  jr ra
ENDPROC(disable_interrupt)

// NOTE: the address of previous pcb in a0
// NOTE: the address of next pcb in a1
ENTRY(switch_to)
  addi sp, sp, -(SWITCH_TO_SIZE)

  /* TODO: [p2-task1] save all callee save registers on kernel stack,
   * see the definition of `struct switchto_context` in sched.h*/

  // # save switch_to_context regs
  sd ra, SWITCH_TO_RA(a0)
  sd sp, SWITCH_TO_SP(a0) 
  sd s0, SWITCH_TO_S0(a0)
  sd s1, SWITCH_TO_S1(a0)
  sd s2, SWITCH_TO_S2(a0)
  sd s3, SWITCH_TO_S3(a0)
  sd s4, SWITCH_TO_S4(a0)
  sd s5, SWITCH_TO_S5(a0)
  sd s6, SWITCH_TO_S6(a0)
  sd s7, SWITCH_TO_S7(a0)
  sd s8, SWITCH_TO_S8(a0)
  sd s9, SWITCH_TO_S9(a0)
  sd s10, SWITCH_TO_S10(a0)
  sd s11, SWITCH_TO_S11(a0)

  /* TODO: [p2-task1] restore all callee save registers from kernel stack,
   * see the definition of `struct switchto_context` in sched.h*/
   # switch current running 
  
  # restore switch_to_context regs
  ld ra, SWITCH_TO_RA(a1)
  ld sp, SWITCH_TO_SP(a1)   # restore stack
  ld s0, SWITCH_TO_S0(a1)
  ld s1, SWITCH_TO_S1(a1)
  ld s2, SWITCH_TO_S2(a1)
  ld s3, SWITCH_TO_S3(a1)
  ld s4, SWITCH_TO_S4(a1)
  ld s5, SWITCH_TO_S5(a1)
  ld s6, SWITCH_TO_S6(a1)
  ld s7, SWITCH_TO_S7(a1)
  ld s8, SWITCH_TO_S8(a1)
  ld s9, SWITCH_TO_S9(a1)
  ld s10, SWITCH_TO_S10(a1)
  ld s11, SWITCH_TO_S11(a1)


  addi sp, sp, SWITCH_TO_SIZE
  jr ra
ENDPROC(switch_to)

ENTRY(ret_from_exception)
  /* TODO: [p2-task3] restore context via provided macro and return to sepc */
  /* HINT: remember to check your sp, does it point to the right address? */
ENDPROC(ret_from_exception)

ENTRY(exception_handler_entry)

  /* TODO: [p2-task3] save context via the provided macro */


  /* TODO: [p2-task3] load ret_from_exception into $ra so that we can return to
   * ret_from_exception when interrupt_help complete.
   */


  /* TODO: [p2-task3] call interrupt_helper
   * NOTE: don't forget to pass parameters for it.
   */


ENDPROC(exception_handler_entry)
