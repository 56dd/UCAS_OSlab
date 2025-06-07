#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stdint.h>
typedef int32_t pid_t;
typedef pid_t pthread_t;

void sys_sleep(uint32_t time);
void sys_yield(void);
void sys_screen_write(char *buff);
void sys_move_cursor(int x, int y);
void sys_reflush(void);
long sys_get_timebase(void);
long sys_get_tick(void);
int sys_mutex_init(int key);
void sys_mutex_acquire(int mutex_idx);
void sys_mutex_release(int mutex_idx);
void sys_set_sche_workload(int position);

/************************************************************/
/* TODO: [P3 task1] ps, getchar */
void sys_ps(void);
int  sys_getchar(void);
void sys_write_ch(char);

/* TODO: [P3 task1] exec, exit, kill waitpid */
// S-core
//pid_t  sys_exec(int id, int argc, uint64_t arg0, uint64_t arg1, uint64_t arg2);
// A/C-core
 pid_t  sys_exec(char *name, int argc, char **argv);

void sys_exit(void);
int  sys_kill(pid_t pid);
int  sys_waitpid(pid_t pid);
pid_t sys_getpid();


/* TODO: [P3 task2] barrier */ 
int  sys_barrier_init(int key, int goal);
void sys_barrier_wait(int bar_idx);
void sys_barrier_destroy(int bar_idx);

/* TODO: [P3 task2] condition */ 
int sys_condition_init(int key);
void sys_condition_wait(int cond_idx, int mutex_idx);
void sys_condition_signal(int cond_idx);
void sys_condition_broadcast(int cond_idx);
void sys_condition_destroy(int cond_idx);

/* TODO: [P3 task2] mailbox */ 
int sys_mbox_open(char * name);
void sys_mbox_close(int mbox_id);
int sys_mbox_send(int mbox_idx, void *msg, int msg_length);
int sys_mbox_recv(int mbox_idx, void *msg, int msg_length);

/* TODO: [P4-task5] shmpageget/dt */
/* shmpageget/dt */
void* sys_shmpageget(int key);
void sys_shmpagedt(void *addr);

/* net send and recv */
int sys_net_send(void *txpacket, int length);
int sys_net_recv(void *rxbuffer, int pkt_num, int *pkt_lens);

/* file system operations */
int sys_mkfs(int force_flag);
int sys_statfs(void);
int sys_cd(char *path);
int sys_mkdir(char *path);
int sys_rmdir(char *path);
int sys_ls(char *path, int option);
int sys_touch(char *path);
int sys_cat(char *path);
int sys_open(char *path, int mode);
int sys_read(int fd, char *buff, int length);
int sys_write(int fd, char *buff, int length);
int sys_close(int fd);
int sys_ln(char *src_path, char *dst_path);
int sys_rm(char *path);
int sys_lseek(int fd, int offset, int whence);
/************************************************************/

#endif
