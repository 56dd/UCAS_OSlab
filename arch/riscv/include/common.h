/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#ifndef INCLUDE_COMMON_H_
#define INCLUDE_COMMON_H_

#include <type.h>

#define REG_DAT     0x00
#define REG_IER     0x01
#define REG_IIR     0x02
#define REG_FCR     0x02
#define REG_LCR     0x03
#define REG_MCR     0x04
#define REG_LSR     0x05
#define REG_MSR     0x06
#define REG_CR      0x08
#define REG_MR      0x09

#define COLOR_RED      "\e[31m"
#define COLOR_GREEN    "\e[32m"
#define COLOR_YELLOW   "\e[33m"
#define COLOR_BLUE     "\e[34m"
#define COLOR_MAGENTA  "\e[35m"
#define COLOR_CYAN     "\e[36m"
#define COLOR_RESET    "\e[0m"

enum FDT_TYPE {
    TIMEBASE,
    SLCR_BADE_ADDR,
    ETHERNET_ADDR,
    PLIC_ADDR,
    NR_IRQS
};

// enter a char into serial port
// use bios printch function
void port_write_ch(char ch);

// enter a message into seraial port
// use bios printstr function
void port_write(char *buf);

// get a char from serial port
// use bios getch function
int port_read_ch(void);

// read blocks from sd card
// use bios bios_sd_read function
int sd_read(unsigned mem_address, unsigned num_of_blocks, unsigned block_id);

/************************************************************/
// write blocks to sd card
// use bios bios_sdwrite function
int sd_write(unsigned mem_address, unsigned num_of_blocks, unsigned block_id);

// set timer
// use bios set_timer function
void set_timer(uint64_t stime_value);

// read flat device tree
// use bios read_fdt function
uint64_t read_fdt(enum FDT_TYPE type);

// write debug information to logfile, this function is realized via qemu dump
// use bios logging function
void qemu_logging(char *str);

// send Inter-Processor Interrupts(IPI)
// use bios send ipi function
void send_ipi(const unsigned long *hart_mask);
/************************************************************/

#endif
