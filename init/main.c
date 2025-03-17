#include <common.h>
#include <asm.h>
#include <asm/unistd.h>
#include <os/loader.h>
#include <os/irq.h>
#include <os/sched.h>
#include <os/lock.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/mm.h>
#include <os/time.h>
#include <sys/syscall.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>
#include <type.h>
#include <csr.h>

extern void ret_from_exception();

// Task info array
task_info_t tasks[TASK_MAXNUM];


static void init_jmptab(void)
{
    volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

    jmptab[CONSOLE_PUTSTR]  = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ]         = (long (*)())sd_read;
    jmptab[SD_WRITE]        = (long (*)())sd_write;
    jmptab[QEMU_LOGGING]    = (long (*)())qemu_logging;
    jmptab[SET_TIMER]       = (long (*)())set_timer;
    jmptab[READ_FDT]        = (long (*)())read_fdt;
    jmptab[MOVE_CURSOR]     = (long (*)())screen_move_cursor;
    jmptab[PRINT]           = (long (*)())printk;
    jmptab[YIELD]           = (long (*)())do_scheduler;
    jmptab[MUTEX_INIT]      = (long (*)())do_mutex_lock_init;
    jmptab[MUTEX_ACQ]       = (long (*)())do_mutex_lock_acquire;
    jmptab[MUTEX_RELEASE]   = (long (*)())do_mutex_lock_release;

    // TODO: [p2-task1] (S-core) initialize system call table.

}

static void init_task_info(int app_info_loc, int app_info_size)
{
    // TODO: [p1-task4] Init 'tasks' array via reading app-info sector
    // NOTE: You need to get some related arguments from bootblock first
    int start_sec, blocknums;
    start_sec = app_info_loc / SECTOR_SIZE;
    blocknums = NBYTES2SEC(app_info_loc + app_info_size) - start_sec;
    int task_info_addr = TASK_INFO_MEM;
    bios_sd_read(task_info_addr, blocknums, start_sec);
    int start_addr = (TASK_INFO_MEM + app_info_loc - start_sec * SECTOR_SIZE);
    uint8_t *tmp = (uint8_t *)(start_addr);
    memcpy((uint8_t *)tasks, tmp, app_info_size);
}

/************************************************************/
static void init_pcb_stack(
    ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
    pcb_t *pcb)
{
     /* TODO: [p2-task3] initialization of registers on kernel stack
      * HINT: sp, ra, sepc, sstatus
      * NOTE: To run the task in user mode, you should set corresponding bits
      *     of sstatus(SPP, SPIE, etc.).
      */
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));


    /* TODO: [p2-task1] set sp to simulate just returning from switch_to
     * NOTE: you should prepare a stack, and push some values to
     * simulate a callee-saved context.
     */
    switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));

}

static void init_pcb(void)
{
    /* TODO: [p2-task1] load needed tasks and init their corresponding PCB */


    /* TODO: [p2-task1] remember to initialize 'current_running' */

}

static void init_syscall(void)
{
    // TODO: [p2-task3] initialize system call table.
}
/************************************************************/

int main(int app_info_loc, int app_info_size)
{
    // Init jump table provided by kernel and bios(ΦωΦ)
    init_jmptab();

    // Init task information (〃'▽'〃)
    init_task_info(app_info_loc, app_info_size);

    // Init Process Control Blocks |•'-'•) ✧
    init_pcb();
    printk("> [INIT] PCB initialization succeeded.\n");

    // Read CPU frequency (｡•ᴗ-)_
    time_base = bios_read_fdt(TIMEBASE);

    // Init lock mechanism o(´^｀)o
    init_locks();
    printk("> [INIT] Lock mechanism initialization succeeded.\n");

    // Init interrupt (^_^)
    init_exception();
    printk("> [INIT] Interrupt processing initialization succeeded.\n");

    // Init system call table (0_0)
    init_syscall();
    printk("> [INIT] System call initialized successfully.\n");

    // Init screen (QAQ)
    init_screen();
    printk("> [INIT] SCREEN initialization succeeded.\n");

    // TODO: [p2-task4] Setup timer interrupt and enable all interrupt globally
    // NOTE: The function of sstatus.sie is different from sie's
    


    // TODO: Load tasks by either task id [p1-task3] or task name [p1-task4],
    //   and then execute them.
    // task 3
    /*int taskid;
    uint64_t entry_addr;
    void (*entry) (void);
    while(1){
        while((taskid=bios_getchar())==-1);
        bios_putchar(taskid);
        taskid -= '0';
        if(taskid>=0 && taskid<=TASK_MAXNUM){
            bios_putchar('\n');
            entry_addr = load_task_img(taskid);
            entry = (void*) entry_addr;
            entry();
        }
    }
    */
    char taskname[16] = "";
    int j=0;
    int tmp;
    uint64_t entry_addr;
    void (*entry) (void);
    while(1){
        while((tmp=bios_getchar())==-1);
        bios_putchar(tmp);
        if(tmp == '#'){
            bios_putchar('\n');
            taskname[j]='\0';
            if(strcmp(taskname,"batch")==0){
                batch();
                j = 0;
                continue;
            }
            entry_addr = load_task_img(taskname);
            if(entry_addr!=0){
                entry = (void*) entry_addr;
                entry();
            }
            j=0;
        }
        else{
            taskname[j++]=tmp;
        }
        
    }

    // Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
    while (1)
    {
        // If you do non-preemptive scheduling, it's used to surrender control
        do_scheduler();

        // If you do preemptive scheduling, they're used to enable CSR_SIE and wfi
        // enable_preempt();
        // asm volatile("wfi");
    }

    return 0;
}
