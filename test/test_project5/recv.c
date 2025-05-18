#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#define MAX_RECV_CNT 32
#define RX_PKT_SIZE 200

static uint32_t recv_buffer[MAX_RECV_CNT * RX_PKT_SIZE];
static int recv_length[MAX_RECV_CNT];

int main(void)
{
    int print_location = 1;
    int iteration = 1;

    while (1)
    {
        sys_move_cursor(0, print_location);
        printf("[RECV] start recv(%d): ", MAX_RECV_CNT);

        int ret = sys_net_recv(recv_buffer, MAX_RECV_CNT, recv_length);
        printf("%d, iteration = %d\n", ret, iteration++);
        char *curr = (char *)recv_buffer;
        for (int i = 0; i < MAX_RECV_CNT; ++i) {
            sys_move_cursor(0, print_location + 1);
            printf("packet %d:\n", i);
            for (int j = 0; j < (recv_length[i] + 15) / 16; ++j) {
                for (int k = 0; k < 16 && (j * 16 + k < recv_length[i]); ++k) {
                    printf("%02x ", (uint32_t)(*(uint8_t*)curr));
                    ++curr;
                }
                printf("\n");
            }
        }
    }

    return 0;
}