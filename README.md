# Pro3

### 任务 1：终端和终端命令的实现

本任务可以简单的分为两个任务，分别是实现shell以及实现一系列新的操作（exec，exit，kill，wait）。

shell的实现很简单，关键在于两点，一个是我们使用一个buff数组，储存将用于解析的命令，而回显的字符串，我们直接使用sys_write_ch(tmp);sys_reflush();两系统调用结合，sys_write_ch(tmp)调用的screen中的screen_write_ch函数，screen有关'\b'回显也写出来了，即屏幕位置往前移，并用空格覆盖之前字符。对于buff中，遇到删除就往前移，遇到回车就结束。具体代码如下：

```
        end = 0;
        // TODO [P3-task1]: call syscall to read UART port
        while((tmp = sys_getchar())==-1);
        // TODO [P3-task1]: parse input
        // note: backspace maybe 8('\b') or 127(delete)
        if (tmp == '\b' || tmp == 127)
        {
            if (ins_pos > 0)
            {
                sys_write_ch(tmp);
                sys_reflush();
                buff[--ins_pos] = '\0';
            }
        }
        else if (tmp == '\n' || tmp == '\r'){
           sys_write_ch('\n');
           sys_reflush();
           end = 1; 
        }
        else{
            sys_write_ch(tmp);
            sys_reflush();
            buff[ins_pos++] = tmp;
        }

        if(end ==0)
            continue;
        // TODO [P3-task1]: ps, exec, kill, clear    
        else{
            buff[ins_pos] = '\0';
        }
        ins_pos = 0;
```

接下来的步骤是解析命令，并执行相应的操作。

我们书写一个parse_args函数来完成这一操作，最后即是，对比每一个指令并执行相应操作，这里并不难，所以就不粘贴代码了。

不过关于shell，我们可以实现更多有趣的功能，但目前还在赶进度，所以暂时没有写。

接下来有关exec，exit，kill，wait四个新功能，这里首先我对之前有关sche，lock等等都进行了修改，之前的代码写的不太好，耦合度太高了，导致添加新功能十分麻烦，所以现在对代码进行了重构，每一个函数的功能是单一的，不需用多个函数结合来完成一项工作，这样代码维护起来会比较容易，再此基础上完成四个函数。

```
pid_t do_exec(char *name, int argc, char *argv[]){  //创建进程，不成功返回0
    char **argv_ptr;
    int index = search_free_pcb();
    if(index==-1)   // 进程数已满，返回
        return 0;
    uint64_t entry_point;
    entry_point=load_task_img(name);
    if(entry_point==0)   // 找不到相应task，返回
        return 0;
    // 创建PCB
    else{
        pcb[index].kernel_sp = (reg_t)(allocKernelPage(1)+PAGE_SIZE);    //分配一页
        pcb[index].user_sp = (reg_t)(allocUserPage(1)+PAGE_SIZE);
        uint64_t user_sp = pcb[index].user_sp;
        pcb[index].pid = task_num + 1; // pid 0 is for kernel
        pcb[index].status = TASK_READY;
        pcb[index].cursor_x = 0;
        pcb[index].cursor_y = 0;
        pcb[index].wait_list.prev = pcb[index].wait_list.next = &pcb[index].wait_list;
        pcb[index].list.prev = pcb[index].list.next = NULL;
        // 参数搬到用户栈
        user_sp -= sizeof(char*) * argc;
        argv_ptr = (char **)user_sp;
        
        for(int i=argc-1; i>=0; i--){
            int len = strlen(argv[i])+1;    //要拷贝'\0'
            user_sp -=len;
            argv_ptr[i] = (char*)user_sp;
            strcpy((char*)user_sp, argv[i]);
        }
        pcb[index].user_sp = (reg_t)ROUNDDOWN(user_sp, 128);    // 栈指针128字节对齐
        //初始化栈，改变入口地址，存储参数
        init_pcb_stack(pcb[index].kernel_sp, pcb[index].user_sp, entry_point, &pcb[index], argc, argv_ptr);
        // 加入ready队列
        add_node_to_q(&pcb[index].list, &ready_queue);
        // 进程数加一
        task_num++;
    }
    return pcb[index].pid;  //返回pid值
}
```

这里大多数逻辑较为清晰简单，注意的是，首先我们这里pcb有一个新的元素，即wait_list，这是用于wait功能的，我们在这里也需要初始化。另外我们有必要将argc，argv参数传递到寄存器中，argc的传递很简单，但是argv的传递需要考虑，首先令argv指向user_sp的低sizeof(char*) * argc位，这是为了储存argc个*argc（是一个字符串的指针），然后我们将每一个argv[i]数组继续指向user_sp低strlen(argv[i])+1位，这些位置储存的就是最后的字符了。最后即init_pcb_stack函数需要修改一下。

关于exit和kill：

```
void do_exit(void){
    current_running->status = TASK_EXITED;
    pcb_release(current_running);
    do_scheduler();
}

int do_kill(pid_t pid){
    for(int i=0; i<NUM_MAX_TASK; i++){
        if(pcb[i].status!=TASK_EXITED && pcb[i].pid==pid){
            // 修改进程状态
            pcb[i].status = TASK_EXITED;
            pcb_release(&pcb[i]);
            // 返回1，表示找到对应进程且将其kill
            return 1;
        }
    }
    return 0;
}
```

这里逻辑容易理解，重要的是pcb_release()这个函数,用处是释放pcb的资源，目前主要包括，pcb所在进程取出当前队列，释放锁，释放所有wait该进程的进程。

```
void pcb_release(pcb_t* p){

    // 将之从原队列删除
    if(current_running->pid != p->pid)
        delete_node_from_q(&(p->list));
    // 释放等待队列的所有进程
    free_block_list(&(p->wait_list));
    // 释放持有的所有锁
    release_all_lock(p->pid);
}
```

```
void release_all_lock(pid_t pid){
    for(int i=0; i<LOCK_NUM; i++){
        if(mlocks[i].pid == pid )
            do_mutex_lock_release(i);
    }
}
```

wait的逻辑如下:

```
int do_waitpid(pid_t pid){
    for(int i=0; i<NUM_MAX_TASK; i++){
        if(pcb[i].pid == pid){
            if(pcb[i].status != TASK_EXITED){
                do_block(&(current_running->list), &(pcb[i].wait_list));
                do_scheduler();
                return pid;
            }
        }
    }
    return 0;
}
```

至此，Task1完成。

### 任务2 实现同步原语：barriers、condition variables

#### barriers

定义barrier的数据结构：

```
typedef struct barrier
{
    int goal;
    int wait_num;
    list_head wait_list;
    int key;
    use_status_t usage;
} barrier_t;
```

```
void init_barriers(void){
    for(int i=0; i <BARRIER_NUM; i++){
        barrs[i].goal=0;
        barrs[i].wait_num=0;
        barrs[i].usage=UNUSED;
        barrs[i].wait_list.prev = barrs[i].wait_list.next = &barrs[i].wait_list; 
    }
}
int do_barrier_init(int key, int goal){
    // 寻找对应key是否已经有对应屏障变量
    for(int i=0; i<BARRIER_NUM; i++){
        if(barrs[i].usage==USING && barrs[i].key==key){ // 找到匹配屏障变量
            barrs[i].goal = goal;
            return i;
        }
    }
    // 寻找空闲屏障变量
    for(int i=0; i<BARRIER_NUM; i++){
        if(barrs[i].usage==UNUSED){ // 找到空闲屏障变量
            barrs[i].key = key;
            barrs[i].goal = goal;
            return i;
        }
    }
    return -1;  // 未找到，返回-1
}
void do_barrier_wait(int bar_idx){
    barrs[bar_idx].wait_num++;
    if(barrs[bar_idx].goal != barrs[bar_idx].wait_num){
        do_block(&current_running->list, &barrs[bar_idx].wait_list);
        do_scheduler();
    }
    else{
        free_block_list(&barrs[bar_idx].wait_list);
        barrs[bar_idx].wait_num=0;
    }
}
void do_barrier_destroy(int bar_idx){
    free_block_list(&barrs[bar_idx].wait_list);
    barrs[bar_idx].key=0;
    barrs[bar_idx].goal=0;
    barrs[bar_idx].usage=UNUSED;
}

```

有关barrier的init，wait和destroy函数的实现，请参考代码，并没有太多需要注释的地方。然后需要在main函数中init初始化barrier。



### 任务3 开启双核并行运行




