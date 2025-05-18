#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

static uint32_t buffer0[256] = {
    0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040,
    0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000,     0x005e0001,
    0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
    0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};
static uint32_t buffer1[256] = {
    0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040,
    0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000,     0x005e0001,
    0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
    0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};
static uint32_t buffer2[256] = {
    0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040,
    0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000,     0x005e0001,
    0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
    0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};
static uint32_t buffer3[256] = {
    0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040,
    0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000,     0x005e0001,
    0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8,
    0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};

static int len[4] = {88, 88, 88, 88};

int main(void)
{
    int print_location = 0;
    uint32_t *addr[4] = {buffer0, buffer1, buffer2, buffer3};

    sys_move_cursor(0, print_location);
    printf("> [SEND] start send package.               \n");

    for(int i = 0; i < 4; i++) {
        sys_net_send(addr[i], len[i]);
        sys_move_cursor(0, print_location);
        printf("> [SEND] totally send package %d/%d !         \n", i + 1, 4);
    }

    return 0;
}