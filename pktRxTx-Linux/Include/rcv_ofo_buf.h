#ifndef __RCV_OFO_BUF_H__
#define __RCV_OFO_BUF_H__

#include <stdint.h>

typedef struct ofo_buf_entry_t {
    struct ofo_buf_entry_t *prev;	// Link the previous node of ofo_buf
    struct ofo_buf_entry_t *next;	// Link the next node of ofo_buf
    uint32_t seq;					// The seq of packet that is out of order
    uint32_t pl_len;				// The length of the payload in this packet
    uint32_t idx;					// The index of the packet in the packet stream
} ofo_buf_entry_t;

void init_rcv_ofo_buf(void);
void free_rcv_ofo_buf(ofo_buf_entry_t *sentry);
void insert_rcv_ofo_buf(uint32_t seq, uint32_t idx, uint32_t pl_len);

static inline void init_buf(ofo_buf_entry_t *sentry)
{
    sentry->prev = sentry;
    sentry->next = sentry;
}

static inline int is_buf_empty(ofo_buf_entry_t *sentry)
{
    return (int)(sentry->next == sentry);
}

static inline void insert_to_buf(ofo_buf_entry_t *entry, \
                                 ofo_buf_entry_t *prev,  \
                                 ofo_buf_entry_t *next)
{
    next->prev = entry;
    prev->next = entry;
    entry->next = next;
    entry->prev = prev;
}

static inline void add_tail_to_buf(ofo_buf_entry_t *new_entry, \
                                   ofo_buf_entry_t *sentry)
{
    insert_to_buf(new_entry, sentry->prev, sentry);
}

static inline void delete_buf_entry(ofo_buf_entry_t *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

#endif  // !__RCV_OFO_BUF_H__