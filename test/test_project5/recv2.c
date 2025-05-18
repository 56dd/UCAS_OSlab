/*
 * recv2: advanced receiver.
 * usage: recv2 -n[NPKT] -l[LLIM] [-c]
 *  [NPKT] is the number of packets to receive;
 *  [LLIM] is the limit of the length of a packet;
 *  [-c] will set the printing format as character.
 */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MAX_RECV_CNT 256
#define RX_PKT_SIZE 200

static uint32_t recv_buffer[MAX_RECV_CNT * RX_PKT_SIZE];
static int recv_length[MAX_RECV_CNT];

int len_lim = RX_PKT_SIZE;      /* Max printing length */
int flag_char = 0;              /* Whether display as `char` */
int recv_cnt = 32;              /* Number of packets to be received once */

int main(int argc, char *argv[])
{
#ifndef S_CORE
    int print_location = 1;
    int iteration = 1;

    while (--argc)
    {
        ++argv;
        if (strcmp("-c", *argv) == 0)
            flag_char = 1;
        else if (strncmp("-l", *argv, 2) == 0)
            len_lim = atoi(*argv + 2);
        else if (strncmp("-n", *argv, 2) == 0)
            recv_cnt = atoi(*argv + 2);
    }

    if (len_lim <= 0 || recv_cnt <= 0 || recv_cnt > MAX_RECV_CNT)
        return 1;

    while (1)
    {
        sys_move_cursor(0, print_location);
        printf("[RECV] start recv(%d): ", recv_cnt);

        int ret = sys_net_recv(recv_buffer, recv_cnt, recv_length);
        printf("%d, iteration = %d\n", ret, iteration++);
        char *curr = (char *)recv_buffer;
        char *next = curr;
        for (int i = 0; i < recv_cnt; ++i) {
            sys_move_cursor(0, print_location + 1);
            printf("packet %d: len %d    \n", i, recv_length[i]);
            next += recv_length[i];
            if (recv_length[i] > len_lim)
                recv_length[i] = len_lim;

            if (flag_char != 0)
                for (int j = 0; j < (recv_length[i] + 47) / 48; ++j) {
                    for (int k = 0; k < 48 && (j * 48 + k < recv_length[i]); ++k) {
                        int c = *(uint8_t*)curr;
                        if (c >= 32 && c <= 126)
                            printf("%c", c);
                        else
                            printf(".");
                        ++curr;
                    }
                    printf("\n");
                }
            else
                for (int j = 0; j < (recv_length[i] + 15) / 16; ++j) {
                    for (int k = 0; k < 16 && (j * 16 + k < recv_length[i]); ++k) {
                        printf("%02x ", (uint32_t)(*(uint8_t*)curr));
                        ++curr;
                    }
                    printf("\n");
                }
            curr = next;
        }
    }
#endif
    return 0;
}
