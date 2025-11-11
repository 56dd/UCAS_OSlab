#include <os/lock.h>

#ifndef SMP_H
#define SMP_H

#define NR_CPUS 2

extern spin_lock_t klock;  // 大内核锁
extern spin_lock_t screen_lock;
extern spin_lock_t sched_lock; 
extern spin_lock_t bios_lock; 

extern uint64_t cpu_id;
extern uint64_t sched_cpu_id;
extern uint64_t screen_cpu_id;
extern uint64_t bios_cpu_id;
extern uint64_t mtux_cpu_id;

void smp_init();
void wakeup_other_hart();
uint64_t get_current_cpu_id();
void lock_kernel();
void unlock_kernel();
void unlock_sched();

#endif /* SMP_H */
