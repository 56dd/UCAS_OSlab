#ifndef STDATOMIC_H
#define STDATOMIC_H

/* from linux/arch/riscv/include/asm/cmpxchg.h and atomic.h */

#include <stdint.h>

typedef volatile uint32_t atomic_uint32_t;
typedef volatile uint64_t atomic_uint64_t;

typedef volatile unsigned int atomic_uint;
typedef volatile int atomic_int;
typedef volatile unsigned long atomic_ulong;
typedef volatile long atomic_long;

static inline uint32_t atomic_load(volatile uint32_t* obj)
{
    uint32_t arg = UINT32_MAX;
    uint32_t ret;
    __asm__ __volatile__ (
        "amoand.w.aqrl %0, %2, %1\n"
        : "=r"(ret), "+A" (*(uint32_t*)obj)
        : "r"(arg)
        : "memory");
    return ret;
}

static inline uint64_t atomic_load_d(volatile uint64_t* obj)
{
    uint64_t arg = UINT64_MAX;
    uint64_t ret;
    __asm__ __volatile__ (
                          "amoand.w.aqrl %0, %2, %1\n"
                          : "=r"(ret), "+A" (*(uint64_t*)obj)
                          : "r"(arg)
                          : "memory");
    return ret;
}

static inline int fetch_add(volatile void* obj, int arg)
{
    uint32_t ret;
    __asm__ __volatile__ (
        "amoadd.w.aqrl %0, %2, %1\n"
        : "=r"(ret), "+A" (*(uint32_t*)obj)
        : "r"(arg)
        : "memory");
    return ret;
}

static inline int fetch_sub(volatile void* obj, int arg)
{
    uint32_t ret;
    arg = 0 - arg;
    __asm__ __volatile__ (
        "amoadd.w.aqrl %0, %2, %1\n"
        : "=r"(ret), "+A" (*(uint32_t*)obj)
        : "r"(arg)
        : "memory");
    return ret;
}

static inline int atomic_exchange(volatile void* obj, int desired)
{
    int ret;
    __asm__ __volatile__ (
        "amoswap.w.aqrl %0, %2, %1\n"
        : "=r"(ret), "+A" (*(uint32_t*)obj)
        : "r"(desired)
        : "memory");
    return ret;
}

static inline long atomic_exchange_d(volatile void* obj, long desired)
{
    uint64_t ret;
    __asm__ __volatile__ (
                          "amoswap.d.aqrl %0, %2, %1\n"
                          : "=r"(ret), "+A" (*(uint64_t*)obj)
                          : "r"(desired)
                          : "memory");
    return ret;
}


#endif /* ATOMIC_H */
