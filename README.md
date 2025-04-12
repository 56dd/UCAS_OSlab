# Pro3

### 前言

本次实验有关细粒度锁部分的实现，个人认为并不优秀，所以请测试非Task5时，回退到Task4之前的内容，想观看Task5之前的内容，也请回退，因为本人加入细粒度锁之后，个人感觉多多少少有点荒谬，当然如果需要查看Task5的内容，也可以看我的更改，可谓非常荒谬。

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

#### condition

关于条件变量的实现，也比较简单，有关init和destroy函数的实现，请参考代码，并没有太多需要注释的地方。关于wait，我们需要在阻塞当前线程的同时，释放线程所持有的锁。signal表示唤醒一个等待该条件变量的线程，broadcast表示唤醒所有等待该条件变量的线程。

```
void do_condition_wait(int cond_idx, int mutex_idx){ 
    // 阻塞在条件变量的等待队列
    current_running->status = TASK_BLOCKED;
    add_node_to_q(&current_running->list, &conds[cond_idx].wait_list);
    do_mutex_lock_release(mutex_idx);   
    do_scheduler();

}
void do_condition_signal(int cond_idx){
    list_node_t* head, *p;
    head = & conds[cond_idx].wait_list;
    p = head->next;
    if(p!=head)
        do_unblock(p);
}
void do_condition_broadcast(int cond_idx){
    free_block_list(&conds[cond_idx].wait_list);
}
```

#### mailbox

有关mailbox的代码也值得思考，但并不是一个十分值得大花时间写的部分，所以直接参照相关代码就可以了。

### 任务3 开启双核并行运行

我们思考双核，这里有两个重点，第一是双核的启动，第二是双核的调度。

先从启动开始考虑，由于我们需要加载两个内核，所以需要两个内核栈，以免两个内核相互影响，以下是我两个内核的内核栈地址：

```
#define KERNEL_STACK	0x50500000
#define S_KERNEL_STACK  0x50600000
```

从bootloader开始，有关从核的启动,分为3部分，首先关闭所有中断，然后将发生例外的入口地址定为kernel，最后开启软件中断（这是因为我们将通过软件中断的方式来唤醒第二个内核），当然最后也需要打开SSTATUS寄存器中对应位。接下来循环等待换醒就好。

```
secondary:
	/* TODO [P3-task3]: 
	 * 1. Mask all interrupts
	 * 2. let stvec pointer to kernel_main
	 * 3. enable software interrupt for ipi
	 */
	// 全局关中断(call disable_interrupt)
	li t0, SR_SIE
  	csrc CSR_SSTATUS, t0

	// 将stvec指向kernel main	
	la t0, kernel
	csrw stvec, t0
	// 允许软件中断
	li t0, SIE_SSIE
	csrs sie, t0		// 开启sie寄存器中对应位
	li t0, SR_SIE
	csrs sstatus, t0	// 开启sstatus寄存器中对应位
	 

wait_for_wakeup:
	wfi
	j wait_for_wakeup
```

接下来是初始化C语言环境，这里包括bss段的清空，然而事实这个只需要由主核来完成就可以了，从核需要做的就是初始化栈指针和tp指针，tp指针的初始化是因为，在之前的代码框架里，我们用了register关键词让tp指向当前线程的pcb，而现在，我无法指定让哪个内核的tp指向哪个pcb，所以我们选择在这里先初始化，让tp指向两个内核的初始pcb0。

```
s_start:
  la tp, s_pid0_pcb
  la sp, S_KERNEL_STACK
  call main
```

接下来是各种全局变量的初始化，这里我们让主核进行上述的初始化，从核直接开始设置定时器中断然后调度就可以了。

这样基本上算是启动了双核。

有关双核的调度。我们为了防止两个核互相影响，所以我们使用一个大锁，每当一个核进入到内核时，就上锁，当一个核退出时，就释放锁。这样保证同时只有一个内核正在内核态运行。这里我们就需要实现一个真正的原子指令自旋锁：

```
void spin_lock_init(spin_lock_t *lock)
{
    lock -> status = UNLOCKED;
}

int spin_lock_try_acquire(spin_lock_t *lock)
{
    return (atomic_swap(LOCKED, &lock->status)==UNLOCKED);
}

void spin_lock_acquire(spin_lock_t *lock)
{
    while(atomic_swap(LOCKED, &lock->status)==LOCKED);
}
```

这里我们使用原子指令atomic_swap来实现，这个函数的功能就是将传入的参数与目标地址的值进行交换，并返回原值。这样就可以保证原子性，只会有一个内核得到锁。

```
int tmp_cpu_id = get_current_cpu_id();
    if(tmp_cpu_id == 0){
        // 初始化大内核锁并上锁
        smp_init();
        lock_kernel();
        ···
        do_exec("shell", 0, NULL);

        // 释放大内核锁，唤醒从核
        unlock_kernel();
        wakeup_other_hart(NULL);
        // 重新抢内核锁
        lock_kernel();
        cpu_id = 0;
    }
    else{
        lock_kernel();
        cpu_id = 1; // 强制置为1，避免出现其id不为1而下标越界的情况
        current_running[cpu_id]->status = TASK_RUNNING; 
    }
```

对于主核，我们要首先初始化大内核锁，然后上锁，然后完成各种初始化，并启动shell，然后唤醒从核，然后与从核一起竞争内核锁。而从核只需要竞争锁，并将初始线程设置为TASK_RUNNING，然后继续运行。

然后我们

setup_exception();

这里有改动，我们需要在该函数中加一句

```
csrw sip, zero
```

这里sip寄存器指示当前有哪些中断源在 S-mode（Supervisor 模式）处于等待（pending）状态。我们清除这个寄存器，也就是说之前发生的软中断就不会再需要处理了，不然的话当解除中断，会立刻发生软中断。

接下来我们设置定时器中断，并打印信息，然后释放大内核锁，唤醒从核，然后等待中断的发生，开始调度。

```
bios_set_timer(get_ticks()+TIMER_INTERVAL);
    if(cpu_id == 0)
        printk("> [INIT] CPU 0 initialization succeeded.\n");
    else 
        printk("> [INIT] CPU 1 initialization succeeded.\n");

    unlock_kernel();
```

接下来，我们要考虑的一个问题是，发生中断后何时上锁？

这里正确的是，保存好所以的寄存器，然后上锁，然后调度，然后释放锁，然后恢复寄存器，然后继续运行。这是因为，我们只有保存了之前的寄存器，才是使用了内核栈上的寄存器们，不然会与用户栈混淆，一定会出现错误。返回时也是。

即：

```
ENTRY(exception_handler_entry)

  /* TODO: [p2-task3] save context via the provided macro */
  SAVE_CONTEXT
  call lock_kernel
```

接下来，我们将之前的所有current_running改为current_running[cpu_id]，这样我们就可以区分主核和从核了。为了区分cpu_id，我们在处理中断函数时，先获取当前的cpu_id。

```
void interrupt_helper(regs_context_t *regs, uint64_t stval, uint64_t scause)
{
    cpu_id = get_current_cpu_id();
    ···
}
```

然后就可以快乐的双核调度了。

当然还有一个要改的地方，那就是我的const ptr_t s_pid0_stack = INIT_KERNEL_STACK + 2 * PAGE_SIZE;那么我们为用户程序分配内核栈，应当从这个位置开始分配。改掉mm.h的#define FREEMEM_KERNEL (INIT_KERNEL_STACK+2*PAGE_SIZE)。

至此双核调度完成，我们发现，双核要写的代码甚至不如前两个Task，但是双核如何启动，如何调度，非常需要我们思考，一招不慎，满盘皆属，而且在双核里，debug也是一个十分具有困难性的工作。更多的是关于双核如何设计，这需要对之前的框架进行一定整改，这个能力是比实现一个单一的功能，更加可贵的。

### 任务 4：shell 命令 taskset————将进程绑定在指定的核上

这其实是一个小需求，相比任务3以及即将要做的任务5，这个需求相对简单，我们只需要在shell中实现一个命令，这个命令的功能就是将进程绑定在指定的核上。我们定义一个新的函数，do_taskset。

```
pid_t do_taskset(int mode_p, int mask, void* pid_name){
    int pid = (int)pid_name;
    if(mode_p){
        for(int i=0; i<NUM_MAX_TASK; i++){
            if(pcb[i].status!=TASK_EXITED && pcb[i].pid == pid){
                pcb[i].cpu_mask = mask;
                return pid;
            }
        }
        printk("Fail to find task with pid %d", pid);
        return 0;
    }
    else{
        char *name = (char*)pid_name;
        pid = do_exec(name, 1, &name);
        for(int i=0; i<NUM_MAX_TASK; i++){
            if(pcb[i].status!=TASK_EXITED && pcb[i].pid == pid)
                pcb[i].cpu_mask = mask;
        }
        return pid;
    }
}
```

因为taskset有两种命令形式，所以我们在函数中也有两种处理方式。

重要的是，这里我们在pcb数据结构中新添加了run_cpu_id和cpu_mask两个变量，run_cpu_id表示当前进程在哪个核上运行，cpu_mask表示当前进程在哪个核上运行。我们有必要维护和初始化这两个变量，关于cpu_mask，我们在两个核的pcb0中均初始化为0x3，代表两个核都可以运行。而后我们在exec函数中，默认子进程沿用父进程的cpu_mask，即子进程在父进程所在的核上运行。然后再taskset中，我们可以修改进程的cpu_mask,另外关于run_cpu_id，每当一个task被调度时，我们就更新run_cpu_id，这样我们就可以知道当前进程在哪个核上运行。

然后修改ps命令，以及在shell中添加taskset命令。即可，这比较简单，直接参考代码即可。

#### 突发bug

这个时候，我在O2测试时突然出现了一个bug，那就是在waitpid我灵机一动kill了正在运行的waitpid进程，然后就直接报错了！

这种bug是相当好de的，我们直接锁定kill，然后调试，果然，是因为双核之后，有关pcb_release的全新逻辑哦，应当是


```
void pcb_release(pcb_t* p){

    // 将之从原队列删除
    if(current_running[0]->pid != p->pid & current_running[1]->pid != p->pid)
        delete_node_from_q(&(p->list));
    // 释放等待队列的所有进程
    free_block_list(&(p->wait_list));
    // 释放持有的所有锁
    release_all_lock(p->pid);
}
```

无论是哪个核正在跑程序，我们都不用将之从原队列删除，因为它现在根本不在队列上。这样的问题，也让我考虑到细粒度锁的实现十分困难，如何让两个核都可以进入内核，这是值得思考的问题。

### 任务 5：细粒度锁内核实现

毫无疑问，细粒度锁是我认为的一个非常非常困难的需求，由于本次实验我们其实也并不用把粒度实现的那么细，所以还是没有那么难，但即使如此也消耗了我一天的设计，一天的debug，它的难并不难在要添加多少代码，而在于设计。我也是借鉴了同级蓝宇舟同学的思路，以至于没有踩一个大坑，但仍然耗费了我很多思路。

这里我们重点分三个细粒度锁，sched锁，bios锁，以及screen锁，从此可见我的锁很潦草，并不细hhh，从某种角度而言，我可以实现更多的锁，让粒度更细，如添加sleep锁，time锁，各种同步原语锁，但是由于我的上述实现都用到了sched，而又因为在getpid这个地方，我无法想到如何设置锁，于是最后，我放弃了，选择了在syscall和handle_irq_timer上根据类型取锁释放锁，关于PS，我发现需要同时拿到sched和screen锁，这里还需要更改的时cpu_id，因为我们的cpu_id在随时发生改变，在任何系统调用中，几乎都要用，我们不能以cpu_id作为锁的粒度，于是我想的办法是，设置更多cpu_id，如sched_cpu_id，screen_cpu_id，bios_cpu_id，让他们分别作为相关系统调用的cpu_id。

当然有一个重点，那就是第一次sched如何释放锁，因为第一次sched我们进入一个假现场，直接进入到ret_from_exception了，而不会再释放锁了，所以我设置一个新的假现场（致敬Lucaslan，这是他的经验）

```
ENTRY(ret_from_exception_v)
  call    unlock_sched
  la      ra, ret_from_exception
  jr      ra
ENDPROC(ret_from_exception_v)
```

然后似乎，就成功完成了细粒度锁了，最难的地方在于debug，因为如果双核同时进入临界区，造成的后果难以预料，无法debug，只能通过理清逻辑，然后修改。关于死锁倒是容易，哪里死了改哪里。

至此Pro3完结，撒花！！！
