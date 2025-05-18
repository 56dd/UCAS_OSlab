#include <asm.h>

#define USER_STACK      0x52500000

.section ".entry_function","ax"
ENTRY(_start)

    /* TODO: [p1-task3] setup C runtime environment for the user program */

    la t0, __bss_start         // bss起始地址
    la t1, __BSS_END__         // bss终止地址
do_clear:  
    sw  zero, 0(t0)          // store 0 to t0
    add t0, t0, 4            // point to next word
    bltu t0, t1, do_clear

    /* TODO: [p1-task3] enter main function */
    jal main

    /* TODO: [p1-task3] finish task and return to the kernel, replace this in p3-task2! */

    /************************************************************/
    /* TODO: [P3] call sys_exit */
    call sys_exit
    /************************************************************/
// while(1) loop, unreachable here
loop:
    wfi
    j loop

END(_start)
