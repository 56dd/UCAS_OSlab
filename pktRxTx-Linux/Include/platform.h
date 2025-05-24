#ifndef __PLATFORM_H__
#define __PLATFORM_H__

int  platform_is_admin(void);

void platform_create_thread(void *param, void *func);
void platform_destroy_thread(void *param);
void platform_wait_thread(void *param);
int  platform_is_current_thread(void *param);

void platform_mutex_init(void *param);
void platform_mutex_destroy(void *param);
void platform_mutex_lock(void *param);
void platform_mutex_unlock(void *param);

void platform_wait_us(int us);

#endif  // !__PLATFORM_H__