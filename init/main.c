#include <common.h>
#include <asm.h>
#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <os/loader.h>
#include <type.h>

#define VERSION_BUF 50

int version = 2; // version must between 0 and 9
char buf[VERSION_BUF];

// Task info array
task_info_t tasks[TASK_MAXNUM];

static int bss_check(void)
{
    for (int i = 0; i < VERSION_BUF; ++i)
    {
        if (buf[i] != 0)
        {
            return 0;
        }
    }
    return 1;
}

static void init_jmptab(void)
{
    volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

    jmptab[CONSOLE_PUTSTR]  = (long (*)())port_write;
    jmptab[CONSOLE_PUTCHAR] = (long (*)())port_write_ch;
    jmptab[CONSOLE_GETCHAR] = (long (*)())port_read_ch;
    jmptab[SD_READ]         = (long (*)())sd_read;
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
/* Do not touch this comment. Reserved for future projects. */
/************************************************************/

int main(int app_info_loc, int app_info_size)
{
    // Check whether .bss section is set to zero
    int check = bss_check();

    // Init jump table provided by kernel and bios(ΦωΦ)
    init_jmptab();

    // Init task information (〃'▽'〃)
    init_task_info(app_info_loc, app_info_size);

    // Output 'Hello OS!', bss check result and OS version
    char output_str[] = "bss check: _ version: _\n\r";
    char output_val[2] = {0};
    int i, output_val_pos = 0;

    output_val[0] = check ? 't' : 'f';
    output_val[1] = version + '0';
    for (i = 0; i < sizeof(output_str); ++i)
    {
        buf[i] = output_str[i];
        if (buf[i] == '_')
        {
            buf[i] = output_val[output_val_pos++];
        }
    }

    bios_putstr("Hello OS!\n\r");
    bios_putstr(buf);
    //读取终端输入并回显
    /*
    int tmp;
    while(1){
        while((tmp=bios_getchar())==-1);
        bios_putchar(tmp);
    }
    */

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
        asm volatile("wfi");
    }

    return 0;
}
