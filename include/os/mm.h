/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                                   Memory Management
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
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
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */
#ifndef MM_H
#define MM_H

#include <type.h>
#include <pgtable.h>
#include <os/sched.h>

#define MAP_KERNEL 1
#define MAP_USER 2
#define MEM_SIZE 32
#define PAGE_SIZE 4096 // 4K
#define INIT_KERNEL_STACK 0xffffffc052000000
#define INIT_USER_STACK 0xffffffc052500000
#define FREEMEM_KERNEL (INIT_KERNEL_STACK+2*PAGE_SIZE)
#define FREEMEM_USER INIT_USER_STACK

/* Rounding; only works for n = power of two */
#define ROUND(a, n)     (((((uint64_t)(a))+(n)-1)) & ~((n)-1))
#define ROUNDDOWN(a, n) (((uint64_t)(a)) & ~((n)-1))

extern ptr_t allocPage(int numPage);
// TODO [P4-task1] */
void freePage(ptr_t baseAddr);
void free_all_pages(pcb_t* pcb);

// #define S_CORE
// NOTE: only need for S-core to alloc 2MB large page
#ifdef S_CORE
#define LARGE_PAGE_FREEMEM 0xffffffc056000000
#define USER_STACK_ADDR 0x400000
extern ptr_t allocLargePage(int numPage);
#else
// NOTE: A/C-core
#define USER_STACK_ADDR 0xf00010000
#endif

// TODO [P4-task1] */
extern void* kmalloc(size_t size);
extern void share_pgtable(uintptr_t dest_pgdir, uintptr_t src_pgdir);
extern uintptr_t alloc_page_helper(uintptr_t va, uintptr_t pgdir);

// TODO [P4-task3] swap page*/
//限制用户可使用的物理空间
extern uintptr_t do_uva2pa(uintptr_t uva);
extern uint64_t image_end_sec;
#define USER_PAGE_MAX_NUM 128     // 用户“看见”的其可使用的页数
#define KERN_PAGE_MAX_NUM 4    // 内核实际可供用户使用的页数
extern int page_cnt;        // 记录当前用户使用的info结点的总数
extern int pgdir_id;        // 标识具体是哪个页表

typedef struct{
    list_node_t lnode;
    uint64_t uva; 
    uint64_t pa;      // uva（原本）对应的pa  
    int on_disk_sec;  // 若被换出，其在磁盘中的位置
    int pgdir_id;
}alloc_info_t;  // 记录供用户使用的内核虚地址分配的信息
alloc_info_t alloc_info[USER_PAGE_MAX_NUM];
extern list_head in_mem_list;
extern list_head swap_out_list;
extern list_head free_list;
extern uintptr_t alloc_limit_page_helper(uintptr_t va, uintptr_t pgdir);
extern void init_uva_alloc();
extern alloc_info_t* swapPage();
extern ptr_t uva_allocPage(int numPage, uintptr_t uva);

static inline int get_pgdir_id(uintptr_t pgdir){
    for(int i=0; i<NUM_MAX_TASK; i++){
        if(pgdir==pcb[i].pgdir)
            return i;
    }
}
static inline alloc_info_t* lnode2info(list_node_t* lnode){
    return (alloc_info_t*)lnode;    // 原因：lnode是info的第一个成员变量，lnode的地址即info的地址
}

// TODO [P4-task4]: shm_page_get/dt */
uintptr_t shm_page_get(int key);
void shm_page_dt(uintptr_t addr);



#endif /* MM_H */
