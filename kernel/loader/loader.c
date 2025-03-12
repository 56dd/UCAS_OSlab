#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <type.h>

uint64_t load_task_img(int taskid)
{
    /**
     * TODO:
     * 1. [p1-task3] load task from image via task id, and return its entrypoint
     * 2. [p1-task4] load task via task name, thus the arg should be 'char *taskname'
     */
    uint64_t entry_addr;
    char info[] = "Loading task _ ...\n\r";
    for(int i=0;i<strlen(info);i++){
        if(info[i]!='_') bios_putchar(info[i]);
        else bios_putchar(taskid +'0');
    }
    entry_addr = TASK_MEM_BASE + TASK_SIZE * (taskid - 1);  
    bios_sd_read(entry_addr, 15, 1 + taskid * 15);
    return entry_addr;

}