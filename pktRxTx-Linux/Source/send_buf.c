#include <stdio.h>
#include <stdlib.h>

#include "common.h"

static char s_data1[] = "\nLet's test Collatz Conjecture on number ";
static char s_data2[] = "\nNext step outcomes ";
static char s_data3[] = "\nIt is done!";

static uint32_t cnumber;
static int this_info;
static int next_info;

static void fill_send_buf()
{
    send_buf_t* send_buf = &g_sender.send_buf;
    char payload[100];
    char number[100];

    if(send_buf->deq_seq == 0 && send_buf->valid_len == 0){
        *(uint32_t*)(send_buf->buf) = g_config.file_size + 4;
        send_buf->valid_len += 4;
    }
    
    if(send_buf->valid_len >= REFILL_THRESHOLD){
        return;
    }

    if(send_buf->fp){
        if(send_buf->file_done){
            return;
        }

        char readbuf[SEND_BUF_SIZE];
        uint32_t ptr = (send_buf->deq_ptr+send_buf->valid_len)%SEND_BUF_SIZE;
        int readbytes = fread(readbuf, 1, SEND_BUF_SIZE-send_buf->valid_len, send_buf->fp);
        if(readbytes != SEND_BUF_SIZE-send_buf->valid_len){
            send_buf->file_done = 1;
            printf("Debug: File read done, len = %d\n", ftell(send_buf->fp));
        }
        for(int i = 0; i < readbytes; i++){
            send_buf->valid_len++;
            send_buf->buf[ptr] = readbuf[i];
            ptr++;
            if(ptr >= SEND_BUF_SIZE){
                ptr = 0;
            }
        }
    }
    else{
        while(1){
            if(this_info == 1){
                cnumber = rand();
                sprintf(number, "%u", cnumber);
                strcpy(payload, s_data1);
                strcat(payload, number);
                next_info = 2;
                if(cnumber == 1){
                    next_info = 3;
                }
            }
            else if(this_info == 2){
                if(cnumber % 2){
                    cnumber = cnumber * 3 + 1;
                }
                else{
                    cnumber = cnumber >> 1;
                    if(cnumber == 1){
                        next_info = 3;
                    }
                }
                sprintf(number, "%u", cnumber);
                strcpy(payload, s_data2);
                strcat(payload, number);
            }
            else if(this_info == 3){
                strcpy(payload, s_data3);
                next_info = 1;
            }
            
            int len = strlen(payload);
            if(len + send_buf->valid_len > SEND_BUF_SIZE){
                break;
            }
            
            uint32_t ptr = (send_buf->deq_ptr+send_buf->valid_len)%SEND_BUF_SIZE;
            for(int i = 0; i < len; i++){
                send_buf->buf[ptr] = payload[i];
                ptr++;
                if(ptr >= SEND_BUF_SIZE){
                    ptr = 0;
                }
            }
            send_buf->valid_len += len;

            this_info = next_info;
        }
    }
}

void init_send_buf(const char * fname)
{
    send_buf_t* send_buf = &g_sender.send_buf;

    if(strlen(fname)){
        send_buf->fp = fopen(fname, "rb");
    }
    else{
        send_buf->fp = NULL;
    }

    send_buf->valid_len = 0;
    send_buf->deq_ptr = 0;
    send_buf->deq_seq = 0;
    send_buf->file_done = 0;

    this_info = 1;

    fill_send_buf();
}

uint32_t read_send_buf(char * packet, uint32_t * len)
{
    send_buf_t* send_buf = &g_sender.send_buf;

    if(*len > REFILL_THRESHOLD){
        printf("[%s] Error: Request too much data (%d bytes)\n", \
                __FUNCTION__, *len);
        exit(EXIT_FAILURE);
    }
    if(*len > send_buf->valid_len){
        *len = send_buf->valid_len;
    }
    send_buf->valid_len -= *len;

    int remain = *len;
    int tail_len = SEND_BUF_SIZE - send_buf->deq_ptr;
    if(remain >= tail_len){
        memcpy(packet, send_buf->buf + send_buf->deq_ptr, tail_len);
        send_buf->deq_ptr = 0;
        remain -= tail_len;
        packet += tail_len;
    }
    memcpy(packet, send_buf->buf + send_buf->deq_ptr, remain);
    send_buf->deq_ptr += remain;

    uint32_t old_seq = send_buf->deq_seq;
    send_buf->deq_seq += *len;

    fill_send_buf();

    return old_seq;
}

uint32_t get_next_send_seq(){
    return g_sender.send_buf.deq_seq;
}



void init_sent_packet_buf(void)
{
    sent_packet_entry_t * sent_packet = &g_sender.sent_packet;
    sent_packet->prev = sent_packet;
    sent_packet->next = sent_packet;
}

void insert_sent_packet_buf(uint32_t seq, char * packet)
{
    sent_packet_entry_t * sent_packet = &g_sender.sent_packet;
    sent_packet_entry_t * new_entry = \
        (sent_packet_entry_t *)malloc(sizeof(sent_packet_entry_t));
    new_entry->seq = seq;
    new_entry->packet = packet;
    new_entry->next = sent_packet;
    new_entry->prev = sent_packet->prev;
    sent_packet->prev->next = new_entry;
    sent_packet->prev = new_entry;
}

char * query_sent_packet_buf(uint32_t seq)
{
    sent_packet_entry_t * sent_packet = &g_sender.sent_packet;
    sent_packet_entry_t * entry;
    printf("Info: Recv RSD of seq=%d\n", seq);
    if(sent_packet->next == sent_packet) printf("But currently all sent packets are ACKed.\n");
    for(entry = sent_packet->next; entry != sent_packet; entry = entry->next){
        if(entry->seq == seq){
            return entry->packet;
        }
        if(entry->seq > seq){
            printf("But no such packet is found. Earliest packet is seq = %d\n", sent_packet->next->seq);
            return NULL;
        }
    }
    return NULL;
}

void ack_sent_packet_buf(uint32_t seq){
    sent_packet_entry_t * sent_packet = &g_sender.sent_packet;
    while(sent_packet->next != sent_packet && sent_packet->next->seq < seq){
        sent_packet_entry_t * to_delete = sent_packet->next;
        sent_packet->next = to_delete->next;
        to_delete->prev = sent_packet;
        free(to_delete->packet);
        free(to_delete);
    }
}