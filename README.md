# Pro2

### 任务 1：任务启动与非抢占式调度

首先我们的pcb结构体为：

```
typedef struct pcb
{
    /* register context */
    // NOTE: this order must be preserved, which is defined in regs.h!!
    reg_t kernel_sp;
    reg_t user_sp;

    /* previous, next pointer */
    list_node_t list;

    /* process id */
    pid_t pid;

    /* BLOCK | READY | RUNNING */
    task_status_t status;

    /* cursor position */
    int cursor_x;
    int cursor_y;

    /* time(seconds) to wake up sleeping PCB */
    uint64_t wakeup_time;

} pcb_t;
```

在main中，我们需要对pcb进行一个初始化，

```
static void init_pcb(void)
{
    /* TODO: [p2-task1] load needed tasks and init their corresponding PCB */
    // PCB for kernel
    uint64_t entry[NUM_MAX_TASK+1];   /* entry of all tasks */
    char needed_tasks[][16] = {
        "print1", "print2", "fly"
    };
    uint64_t entry_addr;
    int tasknum = 0;
    pid0_pcb.status = TASK_RUNNING;
    pid0_pcb.list.prev = NULL;
    pid0_pcb.list.next = NULL;
    init_pcb_stack(pid0_pcb.kernel_sp, pid0_pcb.user_sp, (uint64_t)ret_from_exception, &pid0_pcb);
    // load task by name;
    for(int i= 0; i<3; i++){
        entry_addr = load_task_img(needed_tasks[i]);
        // create a PCB
        if(entry_addr!=0){
            pcb[tasknum].kernel_sp = (reg_t)(allocKernelPage(1)+PAGE_SIZE);    //分配一页
            pcb[tasknum].user_sp = (reg_t)(allocUserPage(1)+PAGE_SIZE);
            pcb[tasknum].pid = tasknum + 1; // pid 0 is for kernel
            pcb[tasknum].status = TASK_READY;
            pcb[tasknum].cursor_x = 0;
            pcb[tasknum].cursor_y = 0;
            init_pcb_stack(pcb[tasknum].kernel_sp, pcb[tasknum].user_sp, entry_addr, &pcb[tasknum]);
            // add to ready queue
            add_node_to_q(&pcb[tasknum].list, &ready_queue);
            
            if(++tasknum > NUM_MAX_TASK)  // total tasks should be less than the threshold
                break;
        }
    }

    /* TODO: [p2-task1] remember to initialize 'current_running' */
    current_running = &pid0_pcb;
}
```

这里，注意在后续实验中，随着测试程序越来越多，需要在needed_tasks中添加对应的程序名，同时，需要修改for循环中的i的上限。

这里，我们定义了一个pcb0的pcb块，目的是为了在第一次调度时，模拟一个上下文切换的过程。

我们需要对每一个pcb进程块进行初始化，目前来说，具体就是分配内核栈块和用户栈块，设置起始状态为READY，并设置pid编号，最后需要初始化进程内核栈的上下文。这里有一个制作假现场的过程，因为在第一次进入到这个进程时，它事实还没有开始执行，所以我们有必要设置一个假现场，就目前不添加中断处理而言，策略就是将ra设置为entry_point,sp设置为用户栈顶，这样就可以完成第一次上下文切换了。具体代码在：

```
switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));  
    pcb->kernel_sp = kernel_stack - sizeof(switchto_context_t) - sizeof(regs_context_t); 
    pt_switchto->regs[0] = (uint64_t)entry_point;     // ra        
    pt_switchto->regs[1] = pcb->user_sp;  // sp
```

接下来就可以描述调度算法了：

```
    pcb_t * prior_running;
    prior_running = current_running;
    
    if(current_running->pid != 0){
        // add to the ready queue
        if(current_running->status == TASK_RUNNING){
            current_running->status = TASK_READY;
            add_node_to_q(&current_running->list, &ready_queue);
        }    
        else if(current_running->status == TASK_BLOCKED)
            add_node_to_q(&current_running->list, &sleep_queue);
    }
    list_node_t* tmp = seek_ready_node();

    current_running = get_pcb_from_node(tmp);
    current_running->status = TASK_RUNNING;

    printl("pid[%d]:is going to running\n",current_running->pid);

    // TODO: [p2-task1] switch_to current_running
    switch_to(prior_running->kernel_sp, current_running->kernel_sp);
    printl("[%d] switch_to success!!!\n", current_running->pid);
    return;
```

目前的策略就是从ready_queue中取出一个节点，然后进行上下文切换，然后对于上一个节点，如果此时状态为RUNNING，则将其状态设置为READY，并加入到ready队列中，等待下一次调度。如果当前节点的状态为BLOCKED，则将其加入到sleep队列中，等待被唤醒。

最后就是使用switch_to进行上下文切换了。

```
ENTRY(switch_to)
  addi sp, sp, -(SWITCH_TO_SIZE)

  /* TODO: [p2-task1] save all callee save registers on kernel stack,
   * see the definition of `struct switchto_context` in sched.h*/

  // # save switch_to_context regs
  sd ra, SWITCH_TO_RA(a0)
  sd sp, SWITCH_TO_SP(a0) 
  sd s0, SWITCH_TO_S0(a0)
  sd s1, SWITCH_TO_S1(a0)
  sd s2, SWITCH_TO_S2(a0)
  sd s3, SWITCH_TO_S3(a0)
  sd s4, SWITCH_TO_S4(a0)
  sd s5, SWITCH_TO_S5(a0)
  sd s6, SWITCH_TO_S6(a0)
  sd s7, SWITCH_TO_S7(a0)
  sd s8, SWITCH_TO_S8(a0)
  sd s9, SWITCH_TO_S9(a0)
  sd s10, SWITCH_TO_S10(a0)
  sd s11, SWITCH_TO_S11(a0)

  /* TODO: [p2-task1] restore all callee save registers from kernel stack,
   * see the definition of `struct switchto_context` in sched.h*/
   # switch current running 
  
  # restore switch_to_context regs
  ld ra, SWITCH_TO_RA(a1)
  ld sp, SWITCH_TO_SP(a1)   # restore stack
  ld s0, SWITCH_TO_S0(a1)
  ld s1, SWITCH_TO_S1(a1)
  ld s2, SWITCH_TO_S2(a1)
  ld s3, SWITCH_TO_S3(a1)
  ld s4, SWITCH_TO_S4(a1)
  ld s5, SWITCH_TO_S5(a1)
  ld s6, SWITCH_TO_S6(a1)
  ld s7, SWITCH_TO_S7(a1)
  ld s8, SWITCH_TO_S8(a1)
  ld s9, SWITCH_TO_S9(a1)
  ld s10, SWITCH_TO_S10(a1)
  ld s11, SWITCH_TO_S11(a1)


  addi sp, sp, SWITCH_TO_SIZE
  jr ra
ENDPROC(switch_to)
```

我的写法较为简洁，直接从该位置存或写寄存器即可。

最后需要使用跳转表实现一些函数的调用，跳转表的函数定义于main中，如果start_code本身没有，可以在main中自行添加。

### 任务 2：互斥锁的实现

我们观察互斥锁的数据结构：

```
typedef struct mutex_lock
{
    spin_lock_t lock;
    list_head block_queue;
    int key;
} mutex_lock_t;
```
它由一个锁和一个队列组成，其中锁是自旋锁，队列是等待队列。所以当多个进程同时申请锁时，一个进程获得锁后，其他进程会被加入到等待队列中，当锁被释放时，等待队列中的进程会被唤醒，并尝试获取锁。

所以在这我们需要实现以下两个函数：

```
void do_block(list_node_t *pcb_node, list_head *queue)
{
    // TODO: [p2-task2] block the pcb task into the block queue
    pcb_t * tmp = get_pcb_from_node(pcb_node);
    tmp->status = TASK_BLOCKED;
    add_node_to_q(pcb_node, queue);
}

void do_unblock(list_node_t *pcb_node)
{
    // TODO: [p2-task2] unblock the `pcb` from the block queue
    delete_node_from_q(pcb_node);
    pcb_t * tmp = get_pcb_from_node(pcb_node);
    tmp->status = TASK_READY;
    add_node_to_q(pcb_node, &ready_queue);
}
```

所以我们获取互斥锁的过程为：

```

void do_mutex_lock_acquire(int mlock_idx)
{
    /* TODO: [p2-task2] acquire mutex lock */
    if(spin_lock_try_acquire(&mlocks[mlock_idx].lock))
        return;
    // 获取锁失败
    do_block(&current_running->list, &mlocks[mlock_idx].block_queue);
    pcb_t *prior_running = current_running;
    current_running  = get_pcb_from_node(seek_ready_node());
    current_running->status = TASK_RUNNING;
    switch_to(prior_running->kernel_sp, current_running->kernel_sp);
}

void do_mutex_lock_release(int mlock_idx)
{
    /* TODO: [p2-task2] release mutex lock */
    list_node_t* p, *head;
    head = &mlocks[mlock_idx].block_queue;
    p = head->next;
    // 阻塞队列为空，释放锁
    if(p==head)
        spin_lock_release(&mlocks[mlock_idx].lock);
    else
        do_unblock(p);
}
```

如果获得互斥锁就直接返回，否则进入阻塞队列，然后进行上下文切换。

如果释放互斥锁，就从阻塞队列中唤醒一个进程。注意这里之所以没有真正释放互斥锁，是因为如果真的释放了，但对于测试来说，已经申请失败了互斥锁，事实上此时互斥锁是没有任何进程得到的，所以不用释放，直接获得就行。

最后我们还需要完成一个初始化互斥锁的函数：

```
int do_mutex_lock_init(int key)
{
    /* TODO: [p2-task2] initialize mutex lock */
    for(int i=0;i<lock_used_num;i++){
        if(mlocks[i].key == key)
            return i;
    }
    mlocks[lock_used_num].key = key;
    return lock_used_num++;
}
```

这是为了避免重复初始化互斥锁，所以需要一个全局变量lock_used_num来记录已经初始化的互斥锁的数量。如果key已经存在，则直接返回对应的索引，否则新建一个互斥锁，并返回索引。

#### 小bug

在O2测试时，出现报错，发现是进入到了一个非对其的地址，这里我认为原因是load时的地址没有对齐，所以需要修改一下。

```
bios_sd_read(TMP_MEM_BASE, tasks[i].block_nums, start_sec);
memcpy((uint8_t *)(uint64_t)(entry_addr), (uint8_t *)(uint64_t)(TMP_MEM_BASE + (tasks[i].start_addr - start_sec*512)), tasks[i].block_nums * 512); 
return entry_addr;
```

我们使用一个TMP地址，先将每次每个扇区拷到这个地址，然后再拷贝到目标地址。

修改后问题解决。

### 任务 3：系统调用

控制状态寄存器：

spec：发生异常的地址（即后续需要返回的地址）
stvec：中断处理函数的入口地址
sie：中断使能寄存器
sstatus： 
scause：区分不同例外的入口

我们首先完成系统调用的初始化，这里syscall是一个函数指针数组，我们对它的一些元素进行初始化。

```
static void init_syscall(void)
{
    // TODO: [p2-task3] initialize system call table.
    syscall[SYSCALL_SLEEP]          = (long (*)())do_sleep;
    syscall[SYSCALL_YIELD]          = (long (*)())do_scheduler;
    syscall[SYSCALL_WRITE]          = (long (*)())screen_write;
    syscall[SYSCALL_CURSOR]         = (long (*)())screen_move_cursor;
    syscall[SYSCALL_REFLUSH]        = (long (*)())screen_reflush;
    syscall[SYSCALL_GET_TIMEBASE]   = (long (*)())get_time_base;
    syscall[SYSCALL_GET_TICK]       = (long (*)())get_ticks;
    syscall[SYSCALL_LOCK_INIT]      = (long (*)())do_mutex_lock_init;
    syscall[SYSCALL_LOCK_ACQ]       = (long (*)())do_mutex_lock_acquire;
    syscall[SYSCALL_LOCK_RELEASE]   = (long (*)())do_mutex_lock_release;
}
```

然后我们需要完善之前的init_pcb_stack函数，这是因为我们需要在kernel_stack存放更多的寄存器，从用户态返回内核态时，我们需要存放所有的寄存器，还需要存放四个csr寄存器（SSTATUS，SEPC， SBADADDR，SCAUSE），同样从内核态返回用户态时，就会重置这些寄存器，所以我们有必要对这些寄存器进行初始化。

```
    regs_context_t *pt_regs =
        (regs_context_t *)(kernel_stack - sizeof(regs_context_t));
    pt_regs->regs[1] = (uint64_t) entry_point;           // ra
    pt_regs->regs[2] = user_stack;                      // sp
    pt_regs->regs[4] = (uint64_t)pcb;                             // tp
    pt_regs->sstatus = SR_SPIE;  // SPIE set to 1
    pt_regs->sepc = (uint64_t)entry_point;
```

接下来我们设置stvec寄存器，它存放的是例外处理的入口地址，这里例外处理的入口地址即exception_handler_entry

```
la t0, exception_handler_entry
csrw stvec, t0
```

接下来，初始化例外处理 init_exception()，要做的就是初始化exc_table，即遇到每一种例外如何处理，以及调用刚刚的设置stvec寄存器的汇编程序，设置stvec寄存器。

接下来我们书写exception_handler_entry

```
  sd sp, PCB_USER_SP(tp)    // store user stack
  ld sp, PCB_KERNEL_SP(tp)  // recover kernel stack
  addi sp, sp, -OFFSET_SIZE
  sd x0, OFFSET_REG_ZERO(sp)

  sd ra, OFFSET_REG_RA(sp)

  sd gp, OFFSET_REG_GP(sp)
  sd tp, OFFSET_REG_TP(sp)

  sd t0, OFFSET_REG_T0(sp)
  sd t1, OFFSET_REG_T1(sp)
  sd t2, OFFSET_REG_T2(sp)

  ld t0, PCB_USER_SP(tp)
  sd t0, OFFSET_REG_SP(sp)  // store user stack

  sd s0, OFFSET_REG_S0(sp)
  sd s1, OFFSET_REG_S1(sp)

  sd a0, OFFSET_REG_A0(sp)
  sd a1, OFFSET_REG_A1(sp)
  sd a2, OFFSET_REG_A2(sp)
  sd a3, OFFSET_REG_A3(sp)
  sd a4, OFFSET_REG_A4(sp)
  sd a5, OFFSET_REG_A5(sp)
  sd a6, OFFSET_REG_A6(sp)
  sd a7, OFFSET_REG_A7(sp)

  sd s2, OFFSET_REG_S2(sp)
  sd s3, OFFSET_REG_S3(sp)
  sd s4, OFFSET_REG_S4(sp)
  sd s5, OFFSET_REG_S5(sp)
  sd s6, OFFSET_REG_S6(sp)
  sd s7, OFFSET_REG_S7(sp)
  sd s8, OFFSET_REG_S8(sp)
  sd s9, OFFSET_REG_S9(sp)
  sd s10, OFFSET_REG_S10(sp)
  sd s11, OFFSET_REG_S11(sp)

  sd t3, OFFSET_REG_T3(sp)
  sd t4, OFFSET_REG_T4(sp)
  sd t5, OFFSET_REG_T5(sp)
  sd t6, OFFSET_REG_T6(sp)


  csrr t0, sstatus
  csrr t1, sepc
  csrr t2, sbadaddr
  csrr t3, scause
  sd t0, OFFSET_REG_SSTATUS(sp)
  sd t1, OFFSET_REG_SEPC(sp)
  sd t2, OFFSET_REG_SBADADDR(sp)
  sd t3, OFFSET_REG_SCAUSE(sp)
```

  sd sp, PCB_USER_SP(tp)    // store user stack
  ld sp, PCB_KERNEL_SP(tp)  // recover kernel stack

能从tp中得到用户栈指针的原因是，

```
register pcb_t * current_running asm("tp");
```

register关键词表示，我们讲current_running这个变量位置放到了tp中，所以我们是可以根据tp来找到kernel_sp的，同时注意，我们一开始初始化的时候，也是将tp保存了kernel_sp的。然后还要注意的是，保存sp也是有讲究的，我们一开始讲user_stack先存到tp偏移位中，后续可找回并放到sp中。最后，几个重要的csr寄存器也是需要保存的。

```
    ld t0, PCB_USER_SP(tp)
  sd t0, OFFSET_REG_SP(sp)  // store user stack
```

最后我们只需要根据stval,scause两个寄存器进入interrupt_helper函数

```
  addi a0, sp, 0
  csrr a1, stval
  csrr a2, scause
  call interrupt_helper
```

在interrupt_helper函数中，我们根据是中断还是异常，分别处理：

```
    if(scause & SCAUSE_IRQ_MASK) // 中断
        irq_table[scause & ~SCAUSE_IRQ_MASK](regs, stval, scause);
    else{
        exc_table[scause & ~SCAUSE_IRQ_MASK](regs, stval, scause);
    }
```

handle_other已经实现，我们不需要管，我们需要实现handle_syscall,这里我们和tiny_libc的syscall库一起书写：

```
void handle_syscall(regs_context_t *regs, uint64_t interrupt, uint64_t cause)
{
    regs->sepc += 4; /* when return from syscall, skip the "ecall" */
    regs->regs[10] = syscall[regs->regs[17]](   /* x17: a7 */
        regs->regs[10],                         /* x10: a0 */
        regs->regs[11],                         /* x11: a1 */
        regs->regs[12],                         /* x12: a2 */
        regs->regs[13],                         /* x13: a3 */
        regs->regs[14]                          /* x14: a4 */
    );
}

static long invoke_syscall(long sysno, long arg0, long arg1, long arg2,
                           long arg3, long arg4)
{
    long res;
    asm volatile(
        "mv     a7, %1\n\t"     /* a7: sysno       */
        "mv     a0, %2\n\t"     /* a0: arg0        */
        "mv     a1, %3\n\t"     /* a1: arg1        */
        "mv     a2, %4\n\t"     /* a2: arg2        */
        "mv     a3, %5\n\t"     /* a3: arg3        */
        "mv     a4, %6\n\t"     /* a4: arg4        */
        "ecall        \n\t"     /* syscall         */
        "mv     %0, a0\n\t"     /* a0:return value */
        :"=r"(res)
        :"r"(sysno), "r"(arg0), "r"(arg1), "r"(arg2), "r"(arg3), "r"(arg4)
    );
    return res;
}
```

这里我们使用内联汇编的方式实现用户库里的syscall。

最后就是 ret_from_exception，代码重复简单，不过多赘述，只需注意最后

```
  sd   sp, PCB_KERNEL_SP(tp)
  ld sp, PCB_USER_SP(tp)
```

保存和恢复sp。

至此，系统调用所有步骤已完成，需注意，初始化内核内pcb进程的切换也需要修改，即初始入口地址为ret_from_exception

```
    switchto_context_t *pt_switchto =
        (switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));  
    pcb->kernel_sp = kernel_stack - sizeof(switchto_context_t) - sizeof(regs_context_t); 
    pt_switchto->regs[0] = (uint64_t)ret_from_exception;     // ra        
    pt_switchto->regs[1] = pcb->kernel_sp;  // sp
```

最后我们书写check_sleeping：

```
void check_sleeping(void)
{
    // TODO: [p2-task3] Pick out tasks that should wake up from the sleep queue
    list_node_t *p, *tmp;
    pcb_t* pcb;
    uint64_t current_time = get_timer();
    for(p=sleep_queue.next; p!=&sleep_queue; p=tmp){
        tmp = p->next;
        pcb = get_pcb_from_node(p);
        if(pcb->wakeup_time <= current_time){
            do_unblock(p);  // wake up process
            add_node_to_q(p, &ready_queue);
        }
    }
}
```

至此，Task3完成。

### 任务 4：定时器中断、抢占式调度

