/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 Regents of the University of California
 */

#include <asm.h>
#include <csr.h>

#define KERNEL_STACK		0x50500000

.section ".entry_function","ax"
ENTRY(_start)
  /* Mask all interrupts */
  csrw CSR_SIE, zero
  csrw CSR_SIP, zero

  /* TODO: [p1-task2] clear BSS for flat non-ELF images */
  la t0, __bss_start
  la t1, __BSS_END__
do_clear:
  sw zero, 0(t0)
  addi t0, t0, 4
  bltu t0, t1, do_clear

  /* TODO: [p1-task2] setup C environment */

  li sp, KERNEL_STACK
  j main


loop:
  wfi
  j loop

END(_start)
