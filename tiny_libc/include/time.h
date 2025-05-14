#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include <unistd.h>

#define CLOCKS_PER_SEC (sys_get_timebase())

typedef uint64_t clock_t;

clock_t clock();

#endif /* TIME_H */
