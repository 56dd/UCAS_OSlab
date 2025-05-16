#include <os/mm.h>
#include <os/string.h>
#include <printk.h>
#include <assert.h>
#include <os/task.h>
#include <pgtable.h>

// NOTE: A/C-core
static ptr_t kernMemCurr = FREEMEM_KERNEL;

#define TOTAL_PAGES 65536         // 总共 1024 页（即 256MB 内存）
#define KERNELMEM_START 0xffffffc050000000lu
#define KERNELMEM_END 0xffffffc060000000lu

#define BITMAP(n) (n - KERNELMEM_START)/(8*PAGE_SIZE)
#define BITMAP_OFFSET(n) ((n - KERNELMEM_START)/(PAGE_SIZE))%8


// 全局变量
static uint8_t page_bitmap[TOTAL_PAGES / 8]; // 位图，每 bit 表示一页的状态

// 初始化内存管理器
void init_memory_manager() {
    bzero(page_bitmap, sizeof(page_bitmap)); // 清零位图
}

bool is_memory_full()
{
    for (int i = 0; i < TOTAL_PAGES / 8; i++) {
        if (page_bitmap[i] != 0xff) {
            return false;
        }
    }
    return true;
}

ptr_t allocPage(int numPage)
{
    // align PAGE_SIZE
    ptr_t ret = ROUND(kernMemCurr, PAGE_SIZE);
    do{
        ret = ROUND(kernMemCurr, PAGE_SIZE);
        kernMemCurr = ret + numPage * PAGE_SIZE;
    }while(page_bitmap[BITMAP(ret)] & (1<<(BITMAP_OFFSET(ret))));

    if(kernMemCurr >= KERNELMEM_END)
    {
        kernMemCurr = FREEMEM_KERNEL;
        if(is_memory_full())
        {
            printk("Memory is full\n");
            assert(0);
        }
    }
    return ret;
}

// NOTE: Only need for S-core to alloc 2MB large page
#ifdef S_CORE
static ptr_t largePageMemCurr = LARGE_PAGE_FREEMEM;
ptr_t allocLargePage(int numPage)
{
    // align LARGE_PAGE_SIZE
    ptr_t ret = ROUND(largePageMemCurr, LARGE_PAGE_SIZE);
    largePageMemCurr = ret + numPage * LARGE_PAGE_SIZE;
    return ret;    
}
#endif

void freePage(ptr_t baseAddr)
{
    // TODO [P4-task1] (design you 'freePage' here if you need):
    if (baseAddr == (ptr_t)NULL) return;

    // 检查地址是否有效
    if (baseAddr < KERNELMEM_START || baseAddr >= KERNELMEM_END) {
        return; // 地址无效
    }

    // 标记该页为空闲
    page_bitmap[BITMAP(baseAddr)] &= ~(1 << (BITMAP_OFFSET(baseAddr)));

}

void free_all_pages(pcb_t* pcb)
{
    PTE *pgd = (PTE*)pcb->pgdir;
    for(int i=0;i<512;i++)
    {
        if(pgd[i] == 0)
        {
            continue;
        }
        PTE *pmd = (uintptr_t *)pa2kva((get_pa(pgd[i])));
        for(int j=0;j<512;j++)
        {
            if(pmd[j] == 0 || (pmd[j] & (_PAGE_READ | _PAGE_WRITE | _PAGE_EXEC)) != 0)
            {
                continue;
            }
            PTE *pte = (uintptr_t *)pa2kva((get_pa(pmd[j])));
            for(int k=0;k<512;k++)
            {
                if(pte[k] == 0)
                {
                    continue;
                }
                freePage(pa2kva(get_pa(pte[k])));
            }
            freePage(pa2kva(get_pa(pmd[j])));
        }
        freePage(pa2kva(get_pa(pgd[i])));
    }
    freePage(pcb->pgdir);
    freePage(pcb->kernel_sp - 8);
}

void *kmalloc(size_t size)
{
    // TODO [P4-task1] (design you 'kmalloc' here if you need):
}


/* this is used for mapping kernel virtual address into user page table */
void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir)
{
    // TODO [P4-task1] share_pgtable:
    memcpy(dest_pgdir, src_pgdir, PAGE_SIZE);
}

/* allocate physical page for `va`, mapping it into `pgdir`,
   return the kernel virtual address for the page
   */
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

// 将虚地址和给定实地址的映射关系存于给定页表，执行成功返回1，否则返回0
int map_page_helper(uintptr_t va, uintptr_t pa, uintptr_t pgdir){
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
        set_attribute(&pgd[vpn2], _PAGE_PRESENT | _PAGE_USER);
        clear_pgdir(pa2kva(get_pa(pgd[vpn2])));
    }
    PTE *pmd = (uintptr_t *)pa2kva((get_pa(pgd[vpn2])));
    if(pmd[vpn1] == 0){
        // 分配一个新的二级页目录
        set_pfn(&pmd[vpn1], kva2pa(allocPage(1)) >> NORMAL_PAGE_SHIFT);
        set_attribute(&pmd[vpn1], _PAGE_PRESENT | _PAGE_USER);
        clear_pgdir(pa2kva(get_pa(pmd[vpn1])));
    }
    PTE *pte = (PTE *)pa2kva(get_pa(pmd[vpn1]));
    // 若pa等于0，即取消映射操作
    if(pa==0){
        pte[vpn0] = 0;
        return 1;
    }
    // 将对应实地址置为pa
    else if(pte[vpn0]==0){
        set_pfn(&pte[vpn0], pa >> NORMAL_PAGE_SHIFT);
        set_attribute(
            &pte[vpn0], _PAGE_PRESENT | _PAGE_READ | _PAGE_WRITE |
                            _PAGE_EXEC | _PAGE_ACCESSED | _PAGE_DIRTY | _PAGE_USER);
        return 1;
    }
    return 0;
}

uintptr_t shm_page_get(int key)
{
    // TODO [P4-task4] shm_page_get:
}

void shm_page_dt(uintptr_t addr)
{
    // TODO [P4-task4] shm_page_dt:
}