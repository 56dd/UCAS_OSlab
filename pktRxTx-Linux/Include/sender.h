#ifndef __SENDER_H__
#define __SENDER_H__

#include <stdint.h>

#ifndef WIN32
#include <pthread.h>
#else
#include <windows.h>
#endif  // !WIN32

#define INTERVAL_SEND        200    // 200 us, default for 'send' command 
#define INTERVAL_BENCHMARK   50     // 50 us, default for 'test' command

#define SHUFFLE_WINDOW 4

typedef struct {
#ifndef WIN32
    pthread_t pid;      // To start a sender thread
#else
    HANDLE pid;         // To start a sender thread
#endif  // !WIN32
    uint32_t send_cnt;  // How many packets sender has sent out
    uint32_t seq;		// Next seq to send
    send_buf_t send_buf;
    sent_packet_entry_t sent_packet;
    int shuffle_pkt_len[SHUFFLE_WINDOW];
    char *shuffle_packet[SHUFFLE_WINDOW];
} sender_t;

extern sender_t g_sender;

#ifndef WIN32
void *sender_thread(void *param);
#else
unsigned int WINAPI sender_thread(void *param);
#endif  // !WIN32

void send_packet_one_epoch(char *payload, int interval, int len);

#endif  // !__SENDER_H__
