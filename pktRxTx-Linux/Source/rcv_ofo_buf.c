#include <stdio.h>
#include <stdlib.h>

#include "common.h"

static ofo_buf_entry_t *allocate_new_entry(uint32_t seq, uint32_t idx, uint32_t len)
{
    ofo_buf_entry_t *new_entry = \
        (ofo_buf_entry_t *)malloc(sizeof(ofo_buf_entry_t));
    if (NULL == new_entry)
    {
        printf("[%s] Error: Malloc for a new ofo_buf entry failed!\n", \
            __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    new_entry->seq = seq;
    new_entry->idx = idx;
    new_entry->pl_len = len;

    return new_entry;
}

static void insert_by_seq(ofo_buf_entry_t *new_entry, ofo_buf_entry_t *sentry)
{
    if (is_buf_empty(sentry))
    {
        add_tail_to_buf(new_entry, sentry);
    }
    else
    {
        // Traverse the rcv_ofo_buf
        int insert_done = 0;

        ofo_buf_entry_t *entry_p = NULL;
        for (entry_p = sentry->next; entry_p != sentry; entry_p = entry_p->next)
        {
            if (new_entry->seq < entry_p->seq)
            {
                insert_to_buf(new_entry, entry_p->prev, entry_p);
                insert_done = 1;
                break;
            }
            else if (new_entry->seq == entry_p->seq)
            {
                g_receiver.dupl_cnt += 1;
                insert_done = 1;
                break;
            }
        }

        if (0 == insert_done)  // All seq in ofo buf is less than new_entry
        {
            add_tail_to_buf(new_entry, &g_receiver.ofo_buf);
        }
    }
}

static void merge_contiguous_pkts(ofo_buf_entry_t *sentry)
{
    ofo_buf_entry_t *entry_p = NULL;
    ofo_buf_entry_t *entry_q = NULL;
    for (entry_p = sentry->next, entry_q = entry_p->next; \
         entry_p != sentry; \
         entry_p = entry_q)
    {
        if (entry_p->seq == g_receiver.next_seq)
        {
            g_receiver.next_seq += entry_p->pl_len;
            g_receiver.next_idx += (int)(entry_p->idx == g_receiver.next_idx);
            delete_buf_entry(entry_p);
            free(entry_p);
        }
        else
        {
            break;
        }
    }
}

void insert_rcv_ofo_buf(uint32_t seq, uint32_t idx, uint32_t len)
{
    // Allocate a new entry of ofo_buf
    ofo_buf_entry_t *new_entry = allocate_new_entry(seq, idx, len);

    // Insert it into ofo_buf by seq (in increasing order)
    platform_mutex_lock(&g_receiver.ofo_mutex);

    insert_by_seq(new_entry, &g_receiver.ofo_buf);

    // Seek for contiguous pkts, and modify the recorded seq in g_receiver
    merge_contiguous_pkts(&g_receiver.ofo_buf);

    platform_mutex_unlock(&g_receiver.ofo_mutex);
}

void init_rcv_ofo_buf(void)
{
    init_buf(&g_receiver.ofo_buf);
    platform_mutex_init(&g_receiver.ofo_mutex);
}

void free_rcv_ofo_buf(ofo_buf_entry_t *sentry)
{
    platform_mutex_lock(&g_receiver.ofo_mutex);

    ofo_buf_entry_t *entry_p = NULL;
    ofo_buf_entry_t *entry_q = NULL;
    for (entry_p = sentry->next, entry_q = entry_p->next; \
         entry_p != sentry; \
         entry_p = entry_q)
    {
        delete_buf_entry(entry_p);
        free(entry_p);
    }

    platform_mutex_unlock(&g_receiver.ofo_mutex);

    platform_mutex_destroy(&g_receiver.ofo_mutex);
}