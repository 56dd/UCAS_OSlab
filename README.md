# Pro4

### 任务一：启用虚拟内存机制并进入内核

本实验是我认为目前最难的一次实验，无论是在想明白实验该如何完成亦或是debug的过程都非常艰辛。关于进入内存，最好的理解办法就是从bootloader开始，完完全全的理一遍。

此处bootblock我们的修改仅是将内核搬到了0x50202000处，然后kernel的入口就是进入到了start.S

然后进入到boot.c:

boot_kernel()函数的目的：

```
int ARRTIBUTE_BOOTKERNEL boot_kernel(unsigned long mhartid)
{
    if (mhartid == 0) {
        setup_vm();
    } else {
        enable_vm();
    }

    /* enter kernel */
    ((kernel_entry_t)pa2kva(_start))(mhartid);

    return 0;
}
```

在主核中开启设置虚存页表：

setup_vm()建立了从0xffffffc050000000 ~ 0xffffffc05fffffff到0x50000000 ~ 0x5fffffff的页表。以及0x50000000 ~ 0x5fffffff到0x50000000 ~ 0x5fffffff的页表。

并开启虚拟内存，即设置satp寄存器。

完成了这些工作我们就可以进入到了内核当中了。

不过注意需要在唤醒从核后取消临时映射。

### 任务一续：执行用户程序

到了现在，才到了虚拟内存真正最难的部分。

我们首先需要在pcb中设置一个新的域，

```
uintptr_t pgdir;
```

用它来记录当前进程的页目录地址。

首先需要修改的就是创建进程的函数exec

首先介绍两个函数：

```
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir)
{
    // TODO [P4-task1] share_pgtable:
    memcpy(dest_pgdir, src_pgdir, PAGE_SIZE);
}

uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir)
{
    // TODO [P4-task1] alloc_page_helper:
    va &= VA_MASK;
    uint64_t vpn2 =
        va >> (NORMAL_PAGE_SHIFT + PPN_BITS + PPN_BITS);
    uint64_t vpn1 = (vpn2 << PPN_BITS) ^
                    (va >> (NORMAL_PAGE_SHIFT + PPN_BITS));
    uint64_t vpn0 = (vpn2 << (PPN_BITS + PPN_BITS)) ^
                    (vpn1 << PPN_BITS) ^
                    (va >> NORMAL_PAGE_SHIFT);
    PTE *pgd = (PTE*)pgdir;
    if (pgd[vpn2] == 0) {
        // 分配一个新的三级页目录，注意需要转化为实地址！
        set_pfn(&pgd[vpn2], kva2pa(allocPage(1)) >> NORMAL_PAGE_SHIFT);
        set_attribute(&pgd[vpn2], _PAGE_PRESENT);
        clear_pgdir(pa2kva(get_pa(pgd[vpn2])));
    }
    PTE *pmd = (uintptr_t *)pa2kva((get_pa(pgd[vpn2])));
    if(pmd[vpn1] == 0){
        // 分配一个新的二级页目录
        set_pfn(&pmd[vpn1], kva2pa(allocPage(1)) >> NORMAL_PAGE_SHIFT);
        set_attribute(&pmd[vpn1], _PAGE_PRESENT);
        clear_pgdir(pa2kva(get_pa(pmd[vpn1])));
    }
    PTE *pte = (PTE *)pa2kva(get_pa(pmd[vpn1]));
    if(pte[vpn0] == 0){
        // 该虚地址从未被分配，分配一个新的页
        ptr_t pa = kva2pa(allocPage(1));
        set_pfn(&pte[vpn0], pa >> NORMAL_PAGE_SHIFT);
    }
    set_attribute(
        &pte[vpn0], _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE |
                        _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER | _PAGE_GLOBAL);
    return pa2kva(get_pa(pte[vpn0]));
}
```

它们的作用分别是将一个页目录复制到另一个页目录上。

以及通过虚地址以及页目录，得到内核虚地址。（同时如果该虚地址在页目录中不存在，将建立相应页表）

所以我们建立一个新的进程的步骤需要新添：

1. 新建页目录（为页目录分配4K空间）
2. 共享内核虚地址
3. 将task加载到该地址中

所以我们更改了之前的loader函数，改为了

```
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
```

需要注意，在内核中我们都需要使用内核虚地址，也只能访问到内核虚地址，所以这就是alloc_page_helper()函数的价值所在。

注意每次switch之前：set_satp:

```
set_satp(SATP_MODE_SV39, current_running[cpu_id]->pid, kva2pa(current_running[cpu_id]->pgdir) >> NORMAL_PAGE_SHIFT);
    local_flush_tlb_all();
    switch_to(prior_running, current_running[cpu_id]);
```

这里我修改了之前的switch_to()函数，这个bug我de了整整一天，原因也在于在虚拟内存下debug难度确实大大加大。

完成这些任务，虚拟内存就完成了。

### 任务二：动态页表和按需调页