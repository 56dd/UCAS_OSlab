#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifndef WIN32
#include <pcap.h>
#else
#include "pcap.h"
#endif  // !WIN32

#include "common.h"

pcap_t *g_handle = NULL;

int main(int argc, char *argv[])
{
    // Check Basic Environment
    printf("Info: pktRxTx was built at %s, %s\n", __DATE__, __TIME__);

    if (1 != platform_is_admin())
    {
        printf("[%s] Error: pktRxTx must run with Admin priviledge!\n", \
            __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    // Load Configuration from argc & argv
    load_config(argc, argv);

    printf("Info: Here, MAC Address is " \
        "%02hhu:%02hhu:%02hhu:%02hhu:%02hhu:%02hhu, " \
        "listening on device %s ...\n", \
        g_config.src_mac[0], g_config.src_mac[1], \
        g_config.src_mac[2], g_config.src_mac[3], \
        g_config.src_mac[4], g_config.src_mac[5], \
        g_config.dev);

    printf("Info: MAC Address of the opposite is " \
        "%02hhu:%02hhu:%02hhu:%02hhu:%02hhu:%02hhu ...\n", \
        g_config.dst_mac[0], g_config.dst_mac[1], \
        g_config.dst_mac[2], g_config.dst_mac[3], \
        g_config.dst_mac[4], g_config.dst_mac[5]);

    // Register signal functions
    if (SIG_ERR == signal(SIGINT, sig_handler))
    {
        printf("[%s] Error: Failed to register SIGINT!\n", \
            __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    // Open the Adapter
    char errbuf[PCAP_ERRBUF_SIZE] = {'\0'};

    g_handle = pcap_open_live(g_config.dev,  /* name of the interface */
                              INT32_MAX,     /* max packets to capture */
                              1,             /* promiscuous mode */
                              1000,          /* read timeout */
                              errbuf);
    if (NULL == g_handle)
    {
        printf("[%s] Error: Cannot open device %s: %s\n", \
            __FUNCTION__, g_config.dev, errbuf);
        exit(EXIT_FAILURE);
    }

    // Create sender and receiver threads according to the mode	
    int mode = g_config.mode;

    init_send_buf(g_config.filename);
    init_sent_packet_buf();

    if (1 == is_sender_mode(mode))
    {
        platform_create_thread(&g_sender.pid, sender_thread);
    }

    if (1 == is_receiver_timer_mode(mode))
    {
        platform_create_thread(&g_receiver.pid, receiver_thread);
        platform_create_thread(&g_timer.pid, timer_thread);
    }

    // Block until those threads exit, and close the adapter
    if (1 == is_sender_mode(mode))
    {
        platform_wait_thread(&g_sender.pid);
    }

    if (1 == is_receiver_timer_mode(mode))
    {
        platform_wait_thread(&g_receiver.pid);
        platform_wait_thread(&g_timer.pid);
    }

    if (NULL != g_handle)
    {
        pcap_close(g_handle);
    }

    return 0;
}
