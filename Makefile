# -----------------------------------------------------------------------
# Project Information
# -----------------------------------------------------------------------

PROJECT_IDX	= 0

# -----------------------------------------------------------------------
# Host Linux Variables
# -----------------------------------------------------------------------

SHELL		= /bin/sh
DIR_OSLAB	= $(HOME)/OSLab-RISC-V
DIR_QEMU	= $(DIR_OSLAB)/qemu

# -----------------------------------------------------------------------
# Build and Debug Tools
# -----------------------------------------------------------------------

CROSS_PREFIX	= riscv64-unknown-linux-gnu-
CC				= $(CROSS_PREFIX)gcc
GDB				= $(CROSS_PREFIX)gdb
QEMU			= $(DIR_QEMU)/riscv64-softmmu/qemu-system-riscv64

# -----------------------------------------------------------------------
# Build/Debug Flags and Variables
# -----------------------------------------------------------------------

CFLAGS			= -O0 -fno-builtin -nostdlib -nostdinc -Wall -mcmodel=medany -ggdb3
USER_CFLAGS		= $(CFLAGS) -Wl,--defsym=TEXT_START=$(USER_ENTRYPOINT) -T riscv.lds

QEMU_OPTS		= -nographic -machine virt -m 256M -kernel $(ELF_USER) -bios none
QEMU_DEBUG_OPT	= -s -S

# -----------------------------------------------------------------------
# UCAS-OS Entrypoints and Variables
# -----------------------------------------------------------------------

USER_ENTRYPOINT			= 0x50000000

# -----------------------------------------------------------------------
# UCAS-OS User Source Files
# -----------------------------------------------------------------------

SRC_USER	= test.S
ELF_USER	= $(patsubst %.S, %, $(SRC_USER))

# -----------------------------------------------------------------------
# Top-level Rules
# -----------------------------------------------------------------------

all: $(SRC_USER) riscv.lds
	$(CC) $(USER_CFLAGS) -o $(ELF_USER) $(SRC_USER) -e main

clean:
	rm -f $(ELF_USER)

gdb:
	$(GDB) $(ELF_USER) -ex "target remote:1234"

run:
	$(QEMU) $(QEMU_OPTS)

debug:
	$(QEMU) $(QEMU_OPTS) $(QEMU_DEBUG_OPT)

.PHONY: all clean gdb run debug
