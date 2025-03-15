#include <asm/biosdef.h>

// os size location (os_size could be sectors [p1-task3] or bytes [p1-task4])
.equ os_size_loc, 0x502001fc

// kernel address (move kernel to here, and jmp here to start kernel)
.equ kernel, 0x50201000

// BIOS function entry (jump here to use BIOS APIs)
.equ bios_func_entry, 0x50150000

// start address for task info is stored in this loacation, size of array is in following 4 bytes
.equ app_info_addr_loc, 0x502001f4

.text
.global main

main:
	// fence on all memory and I/O
	fence

	// TODO: [p1-task1] call BIOS to print string "It's bootblock!"
	la a0, msg
	li a7, BIOS_PUTSTR
	jal bios_func_entry

	// TODO: [p1-task2] call BIOS to read kernel in SD card
	li a7, BIOS_SDREAD
	la a0, kernel
	la t0, os_size_loc
	lh a1, 0(t0)
	li a2, 1
	jal bios_func_entry



	// TODO: [p1-task4] load task-related arguments and pass them to kernel
	la t1, app_info_addr_loc	
	lw a0, (t1)		// pass the location for task info as parameter 1
	lw a1, 4(t1)	// pass the size as parameter 2

	// TODO: [p1-task2] jump to kernel to start UCAS-OS
	jal kernel


	/************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/
// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's Zhu Xuyuan's bootloader...\n\r"

