#ifndef __SEND_BUF_H__
#define __SEND_BUF_H__

#include <stdint.h>

#define SEND_BUF_SIZE 20000
#define REFILL_THRESHOLD 10000

typedef struct send_buf_t {
    FILE *fp;
    char buf[SEND_BUF_SIZE];
    uint32_t valid_len;
    uint32_t deq_ptr;
    uint32_t deq_seq;
    uint32_t file_done;
} send_buf_t;

void init_send_buf(const char * fname);
uint32_t read_send_buf(char * packet, uint32_t * len);
uint32_t get_next_send_seq();

typedef struct sent_packet_entry_t {
    struct sent_packet_entry_t *prev;
    struct sent_packet_entry_t *next;
    uint32_t seq;
    char * packet;
} sent_packet_entry_t;

void init_sent_packet_buf(void);
void insert_sent_packet_buf(uint32_t seq, char * packet);
char * query_sent_packet_buf(uint32_t seq);
void ack_sent_packet_buf(uint32_t seq);

#endif  // __SEND_BUF_H__