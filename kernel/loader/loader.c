#include <os/kernel.h>
#include <os/task.h>
#include <os/string.h>
#include <type.h>
#include <os/mm.h>

/* uint64_t load_task_img(int taskid)
{
    /**
     * TODO:
     * 1. [p1-task3] load task from image via task id, and return its entrypoint
     * 2. [p1-task4] load task via task name, thus the arg should be 'char *taskname'
     **//*
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
*/

// [p1-task4]
uint64_t load_task_img(char *taskname){
    int i;
    uint64_t entry_addr;
    int start_sec;
    for(i=0;i<TASK_MAXNUM;i++){
        if(strcmp(taskname, tasks[i].task_name)==0){
            entry_addr = TASK_MEM_BASE + TASK_SIZE * i;
            start_sec = tasks[i].start_addr / 512;                      // 起始扇区：向下取整
            bios_sd_read(TMP_MEM_BASE, tasks[i].block_nums, start_sec);
            memcpy((uint8_t *)(uint64_t)(entry_addr), (uint8_t *)pa2kva((uint64_t)(TMP_MEM_BASE + (tasks[i].start_addr - start_sec*512))), tasks[i].p_memsz); 
            return entry_addr;  // 返回程序存储的起始位置
        }
    }
    // 匹配失败，提醒重新输入
    char *output_str = "Fail to find the task! Please try again!";
    for(i=0; i<strlen(output_str); i++){
        bios_putchar(output_str[i]);
    }
    bios_putchar('\n');
    return 0;
}


// 将任务映射到用户空间USER_ENTRYPOINT
uint64_t map_task(char *taskname, uintptr_t pgdir){
    int i;
    int start_sec;
    uint64_t entry_addr;
    uint64_t user_va, user_va_end;
    for(i=0;i<TASK_MAXNUM;i++){
        if(strcmp(taskname, tasks[i].task_name)==0){
            start_sec = tasks[i].start_addr / 512;                      // 起始扇区：向下取整
            bios_sd_read(TMP_MEM_BASE, tasks[i].block_nums, start_sec);
            user_va_end = USER_ENTRYPOINT + tasks[i].p_memsz;
            for(user_va = USER_ENTRYPOINT; user_va< user_va_end; user_va += PAGE_SIZE){
                alloc_page_helper(user_va, pgdir);
            }
            entry_addr = alloc_page_helper(USER_ENTRYPOINT, pgdir);
            memcpy((uint8_t *)(uint64_t)(entry_addr), (uint8_t *)pa2kva((uint64_t)(TMP_MEM_BASE + (tasks[i].start_addr - start_sec*512))), tasks[i].p_memsz); 
            return USER_ENTRYPOINT;  // 返回用户虚地址
        }
    }
    // 匹配失败，提醒重新输入
    char *output_str = "Fail to find the task! Please try again!";
    for(i=0; i<strlen(output_str); i++){
        bios_putchar(output_str[i]);
    }
    bios_putchar('\n');
    return 0;
}

void batch(){
    uint64_t entry_addr;
    void (*entry) (void);
    char taskname[16] = "number";
    entry_addr = load_task_img(taskname);
    if(entry_addr!=0){
    entry = (void*) entry_addr;
    entry();
    }
    memcpy(taskname, "mul3", 5);
    entry_addr = load_task_img(taskname);
    if(entry_addr!=0){
    entry = (void*) entry_addr;
    entry();
    }
    memcpy(taskname, "add10", 6);
    entry_addr = load_task_img(taskname);
    if(entry_addr!=0){
    entry = (void*) entry_addr;
    entry();
    }
    memcpy(taskname, "square", 7);
    entry_addr = load_task_img(taskname);
    if(entry_addr!=0){
    entry = (void*) entry_addr;
    entry();
    }
}