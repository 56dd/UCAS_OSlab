#include <os/list.h>
#include <os/lock.h>
#include <os/sched.h>
#include <os/time.h>
#include <os/mm.h>
#include <screen.h>
#include <printk.h>
#include <assert.h>

pcb_t pcb[NUM_MAX_TASK];
const ptr_t pid0_stack = INIT_KERNEL_STACK + PAGE_SIZE;
pcb_t pid0_pcb = {
    .pid = 0,
    .kernel_sp = (ptr_t)pid0_stack,
    .user_sp = (ptr_t)pid0_stack
};

LIST_HEAD(ready_queue);
LIST_HEAD(sleep_queue);

/* global process id */
pid_t process_id = 1;
int fly_num = 0;
int table_p = 0;
int if_switch = 0;

void do_scheduler(void)
{
    // TODO: [p2-task3] Check sleep queue to wake up PCBs
    check_sleeping();

    /************************************************************/
    /* Do not touch this comment. Reserved for future projects. */
    /************************************************************/

    // TODO: [p2-task1] Modify the current_running pointer.

    printl("Scheduler start \n");

    pcb_t * prior_running;
    prior_running = current_running;

    if(current_running->time_slice_remain>0){
        if_switch = 0;
    }
    else if(current_running->time_slice_remain==0){
        current_running->time_slice_remain+=current_running->time_slice;
        if_switch = 1;
    }
    if(if_switch == 1){
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
        current_running->time_slice_remain--;
    }
    else if(if_switch == 0)
    {
        current_running->time_slice_remain--;
    }


    printl("pid[%d]:is going to running\n",current_running->pid);

    // TODO: [p2-task1] switch_to current_running
    switch_to(prior_running->kernel_sp, current_running->kernel_sp);
    printl("[%d] switch_to success!!!\n", current_running->pid);
    return;

}

void do_sleep(uint32_t sleep_time)
{
    // TODO: [p2-task3] sleep(seconds)
    // NOTE: you can assume: 1 second = 1 `timebase` ticks
    // 1. block the current_running
    current_running->status = TASK_BLOCKED;
    // 2. set the wake up time for the blocked task
    current_running->wakeup_time = get_timer()+sleep_time;
    // 3. reschedule because the current_running is blocked.
    do_scheduler();
}

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

list_node_t* seek_ready_node(){
    list_node_t *p = ready_queue.next;
    // delete p from queue
    delete_node_from_q(p);
    return p;
}

void add_node_to_q(list_node_t* node,list_head *head){
    list_node_t *p = head->prev; // tail ptr
    p->next = node;
    node->prev = p;
    node->next = head;
    head->prev = node;           // update tail ptr    
}

void delete_node_from_q(list_node_t* node){
    list_node_t* p, *q;
    p = node->prev;
    q = node->next;
    p->next = q;
    q->prev = p;
    node->next = node->prev = NULL; // delete the node completely
}

pcb_t * get_pcb_from_node(list_node_t* node){
    for(int i=0;i<NUM_MAX_TASK;i++){
        if(node == &pcb[i].list)
            return &pcb[i];
    }
    return &pid0_pcb;    // fail to find the task, return to kernel
}


void do_set_sche_workload(int position){
    if(current_running -> if_fly == 0){
        current_running -> if_fly = 1;
        current_running -> fly_id = fly_num++;
    }
    current_running -> position_last = current_running -> position_now;
    current_running -> position_now = position;
    current_running -> time_last = current_running -> time_now;
    current_running -> time_now = get_ticks();
    printl("pid[%d]:position_last:%d,position_now:%d,time_last:%d,time_now:%d\n",current_running->pid,current_running->position_last,current_running->position_now,current_running->time_last,current_running->time_now);
    if(current_running -> position_last!=0 && (current_running -> position_now < current_running -> position_last))
    {
        current_running -> fly_speed_absolute_b = (current_running -> time_now - current_running -> time_last) / (current_running -> position_last - current_running -> position_now);
        printl("pid[%d]fly_id[%d]:fly_speed_absolute_b:%d\n",current_running->pid,current_running ->fly_id,current_running->fly_speed_absolute_b);
        FLY_SPEED_TABLE[current_running -> fly_id] = current_running -> fly_speed_absolute_b;
        if(current_running -> fly_id >= table_p){
            table_p ++;
        }
        for(int i=0;i<table_p;i++)
        {
            printl("FLT_SPEED_TABLE[%d]:%d\n",i,FLY_SPEED_TABLE[i]);
        }
        current_running -> fly_speed_ralative_b = normalize_speed_table(FLY_SPEED_TABLE, table_p, current_running -> fly_id);
        if(current_running -> fly_speed_ralative_b > 0)
            current_running -> time_slice ++;
        else {
            current_running ->fly_speed_ralative_b = -current_running -> fly_speed_ralative_b;
            current_running -> time_slice --;
            if(current_running -> time_slice < 1)
                current_running -> time_slice = 1;
        }
        printl("pid[%d]:fly_speed_ralative_b:%d,time_slice:%d\n",current_running->pid,current_running->fly_speed_ralative_b,current_running->time_slice);
    }
}

/*
void do_set_sche_workload(int position){
    if(current_running -> if_fly == 0){
        current_running -> if_fly = 1;
        current_running -> fly_id = fly_num++;
        current_running -> position_last = position;
        current_running -> position_now = 0;
        return;
    }
    if(current_running -> position_last!=0 )
    {
        current_running -> position_now ++;
        FLY_LENGTH_TABLE[current_running -> fly_id] = current_running -> position_now;
        if(current_running -> fly_id >= table_p){
            table_p ++;
        }
        if(table_p < fly_num)
            current_running -> time_slice = 20;
        else
            current_running -> time_slice = calculate_time_slice(FLY_LENGTH_TABLE, table_p, current_running -> fly_id);
        printl("pid[%d]:time_slice[%d]",current_running->pid,current_running->time_slice);
    }
}
*/

int calculate_time_slice(int* D_table, int table_p, int fly_id) {
    if (table_p <= 0 || fly_id < 0 || fly_id >= table_p) return -1;

    const int epsilon = 0;        // 等效浮点 ε=0.01 (scale=100)
    const int T_min = 1;          // 降低最小时间片至1
    const int Total_T = 100;
    const int scale = 100;        // 提高精度缩放因子

    // 1. 计算平均进度（允许向下取整误差）
    int sum_D = 0;
    for (int i = 0; i < table_p; i++) sum_D += D_table[i];
    int D_avg = sum_D / table_p;

    // 2. 计算权重：领先进程权重=0，落后进程权重=Δ*scale + ε
    int weights[table_p];
    int sum_weights = 0;
    
    for (int i = 0; i < table_p; i++) {
        int delta = D_avg - D_table[i];
        weights[i] = (delta > 0) ? (delta * scale + epsilon) : 0; // 领先进程无基础权重
        sum_weights += weights[i];
    }

    // 3. 时间片分配（四舍五入）
    int time_slice;
    if (sum_weights > 0) {
        time_slice = (weights[fly_id] * Total_T + sum_weights/2) / sum_weights;
    } else {
        // 所有进程领先：强制均分且不低于T_min
        time_slice = (Total_T + table_p/2) / table_p;
    }

    // 4. 动态最小时间片（领先进程更严格限制）
    int dynamic_T_min = T_min;
    if (D_table[fly_id] > D_avg) { // 领先进程额外惩罚
        dynamic_T_min = (T_min > 1) ? T_min-1 : 1;
    }
    if (time_slice < dynamic_T_min) time_slice = dynamic_T_min;

    // 5. 确保总和不超过100（需全局协调）
    return (time_slice > Total_T) ? Total_T : time_slice;
}


int normalize_speed_table(int* speed_table, int table_p, int fly_id) {
    // 参数有效性检查
    if (!speed_table || table_p <= 0) return false;

    // 查找最小速度值
    int speed_average = 0;
    for (int i = 0; i < table_p; i++) {
        speed_average += speed_table[i];
    }

    speed_average /= table_p;

    // 处理无效最小值（非正数）
    if (speed_average <= 0) return false;

    if(speed_table[fly_id] > speed_average ){
        printl("speed_average:%d,speed_table[%d]:%d\n",speed_average,fly_id,speed_table[fly_id]);
        return speed_table[fly_id]  /speed_average;
    }else{
        printl("speed_average:%d,speed_table[%d]:%d\n",speed_average,fly_id,speed_table[fly_id]);
        return -speed_average  /speed_table[fly_id];
    }
}