#ifndef __INCLUDE_LOADER_H__
#define __INCLUDE_LOADER_H__

#include <type.h>

uint64_t load_task_img(char* task_name);
uint64_t map_task(char *taskname, uintptr_t pgdir);

#endif