#include <os/lock.h>
#include <os/sched.h>
#include <os/list.h>
#include <atomic.h>

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
    if(lock->status == UNLOCKED){
        lock->status = LOCKED;
        return 1;
    }
    return 0;
}

void spin_lock_acquire(spin_lock_t *lock)
{
    /* TODO: [p2-task2] acquire spin lock */
    while(lock->status == LOCKED);
    lock->status = LOCKED;
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
    if(p==head){
        mlocks[mlock_idx].pid = -1;
        spin_lock_release(&mlocks[mlock_idx].lock);
    }
    else{
        mlocks[mlock_idx].pid = get_pcb_from_node(p)->pid;
        do_unblock(p);
    }
}
