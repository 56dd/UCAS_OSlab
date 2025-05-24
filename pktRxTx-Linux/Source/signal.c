#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "common.h"

void sig_handler(int signo)
{
    if (SIGINT == signo)
    {
        printf("Info: Receive SIGINT signal ...\n");

        if (1 == is_receiver_timer_mode(g_config.mode))
        {
            print_results();
            unset_timer();
            free_rcv_ofo_buf(&g_receiver.ofo_buf);
        }

        if (NULL != g_handle)
        {
            pcap_close(g_handle);
        }

        printf("Info: Program exits ...\n");
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("[%s] Error: Invalid signo %d! Program exits ...\n", \
                __FUNCTION__, signo);
        exit(EXIT_FAILURE);
    }
}