#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <atomic.h>
#include <printk.h>

int lock_used_num = 0;
void init_locks(void)
{
    /* TODO: [p2-task2] initialize mlocks */
    for(int i=0; i<LOCK_NUM; i++){
        spin_lock_init(&mlocks[i].lock);
        mlocks[i].block_queue.prev = mlocks[i].block_queue.next = & mlocks[i].block_queue;    // initialize block_queue
        mlocks[i].key = -1;
    }
}

void spin_lock_init(spin_lock_t *lock)
{
    /* TODO: [p2-task2] initialize spin lock */
    lock -> status = UNLOCKED;
}

int spin_lock_try_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] try to acquire spin lock */
    return (atomic_swap(LOCKED, &lock->status)==UNLOCKED);
}

void spin_lock_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] acquire spin lock */
    while(atomic_swap(LOCKED, &lock->status)==LOCKED);
}

void spin_lock_release(spin_lock_t *lock)
{
    /* TODO: [p2-task2] release spin lock */
    lock->status = UNLOCKED;
}

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

void do_mutex_lock_acquire(int mlock_idx)
{
    /* TODO: [p2-task2] acquire mutex lock */
    if(spin_lock_try_acquire(&mlocks[mlock_idx].lock)){
        mlocks[mlock_idx].pid = current_running->pid;
        return;
    }
    // 获取锁失败
    do_block(&current_running->list, &mlocks[mlock_idx].block_queue);
    do_scheduler();
}

void do_mutex_lock_release(int mlock_idx)
{
    /* TODO: [p2-task2] release mutex lock */
    list_node_t* p, *head;
    head = &mlocks[mlock_idx].block_queue;
    p = head->next;
    // 阻塞队列为空，释放锁
    if(p==head){
        mlocks[mlock_idx].pid = -1;
        spin_lock_release(&mlocks[mlock_idx].lock);
    }
    else{
        mlocks[mlock_idx].pid = get_pcb_from_node(p)->pid;
        do_unblock(p);
    }
}


//--------------------------------------------Barrier Interface------------------------------------
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

