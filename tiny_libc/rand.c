#include <stdlib.h>

static int x = 0;

void srand(uint32_t seed)
{
    x = seed;
}

int rand(void)
{
    uint64_t tmp = 0x5deece66dll * x + 0xbll;
    x = tmp & 0x7fffffff;
    return x;
}
