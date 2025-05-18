#include <time.h>
#include <mailbox.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_LENGTH (4*(MAX_MBOX_LENGTH))
char ring_buffer[BUFFER_LENGTH];
volatile int ring_buffer_head, ring_buffer_tail;
void ring_buffer_init()
{
    ring_buffer_head = 0;
    ring_buffer_tail = 0;
}
static inline int ring_buffer_full()
{
    return (
        ((ring_buffer_tail + 1) % BUFFER_LENGTH)
        == ring_buffer_head);
}
static inline int ring_buffer_empty()
{
    return (ring_buffer_head == ring_buffer_tail);
}
void ring_buffer_append(char ch)
{
    while (ring_buffer_full());

    ring_buffer[ring_buffer_tail] = ch;
    ring_buffer_tail = (ring_buffer_tail + 1) % BUFFER_LENGTH;
}
char ring_buffer_pop()
{
    while (ring_buffer_empty());

    char ret = ring_buffer[ring_buffer_head];
    ring_buffer_head = (ring_buffer_head + 1) % BUFFER_LENGTH;
    return ret;
}
void get_str_from_ring_buffer(char* buf, int len)
{
    buf[len] = '\0';
    while (len--) {
        buf[len] = ring_buffer_pop();
    }
}
void put_str_into_ring_buffer(char* buf, int len)
{
    int i;
    for (i = 0; i < len; ++i) {
        ring_buffer_append(buf[i]);
    }
}

void display_usage()
{
    printf("Usage: mailbox <id>\n");
    printf("  <id> : the id of this instance.\n");
    printf("         valid value: a, b, c\n");
    printf("Example:\n");
    printf("> exec mailbox a\n");
}

int output_position(char id, int is_recv)
{
    // id = 'a','b' or 'c'
    // output position:
    //   1: id = a, send_thread
    //   2: id = a, recv_thread
    //   3: id = b, send_thread
    //   4: id = b, recv_thread
    //   5: id = c, send_thread
    //   6: id = c, recv_thread
    // 2*id(send thread) or 2*id + 1(recv thread)
    return (id - 'a' + 1) * 2 + (is_recv & 1);
}

const char mailbox_id_template[] = "mailbox-test-_";
char my_mailbox_id[sizeof(mailbox_id_template)];
char other_mailbox_id[2][sizeof(mailbox_id_template)];
char other_id[2];
void fill_mailbox_id(char my_id)
{
    char id_i;
    int idx = 0;

    strcpy(my_mailbox_id, mailbox_id_template);
    // replace '_' with `id`
    my_mailbox_id[sizeof(mailbox_id_template) - 2] = my_id;

    for (id_i = 'a'; id_i < 'd'; ++id_i) {
        if (id_i != my_id) {
            strcpy(other_mailbox_id[idx], mailbox_id_template);
            other_mailbox_id[idx][sizeof(mailbox_id_template) - 2] = id_i;
            other_id[idx] = id_i;
            ++idx;
        }
    }
}

void fill_buffer()
{
    int i;
    static char chars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (i = 0; i < BUFFER_LENGTH - 1; ++i) {
        ring_buffer_append(chars[i % sizeof(chars)]);
    }
}

void recv_thread(void *arg)
{
    char id = (unsigned long) arg;
    int position = output_position(id, 1);
    char recv_buf[MAX_MBOX_LENGTH] = {0};
    int mq = sys_mbox_open(my_mailbox_id);
    long bytes[3] = {0};
    int len, i;

    sys_move_cursor(1, position);
    printf("[%c-recv] started\n", id);
    sys_sleep(1);

    for (;;)
    {
        len = rand() % (MAX_MBOX_LENGTH / 4 - 2) + 1;
        sys_mbox_recv(mq, recv_buf, len*2);
        for (i = 0; i < len; ++i) {
            ++bytes[recv_buf[i * 2] - 'a'];
            recv_buf[i] = recv_buf[i * 2 + 1];
        }
        put_str_into_ring_buffer(recv_buf, len);

        sys_move_cursor(1, position);
        printf("[%c-recv] from %c: %ld \t from %c: %ld\n",
            id, other_id[0], bytes[other_id[0] - 'a'],
            other_id[1], bytes[other_id[1] - 'a']);
        sys_sleep(1);
    }
}

void send_thread(void *arg)
{
    char id = (unsigned long) arg;
    int position = output_position(id, 0);
    int i, len, target = 0;
    long bytes[2] = {0};
    char send_buf[MAX_MBOX_LENGTH] = {0};

    int mq[2] = {0};
    for (i = 0; i < 2; ++i) {
        mq[i] = sys_mbox_open(other_mailbox_id[i]);
    }

    sys_move_cursor(1, position);
    printf("[%c-send] started\n", id);
    sys_sleep(1);

    for (;;)
    {
        len = rand() % (MAX_MBOX_LENGTH / 4 - 2) + 1;
        target = rand() % 2;
        get_str_from_ring_buffer(send_buf, len);
        for (i = len - 1; i >= 0; --i) {
            send_buf[i * 2 + 1] = send_buf[i];
            send_buf[i * 2] = id;
        }
        sys_mbox_send(mq[target], send_buf, 2*len);
        bytes[target] += len;

        sys_move_cursor(1, position);
        printf("[%c-send] to %c: %ld \t to %c: %ld\n",
            id, other_id[0], bytes[0],
            other_id[1], bytes[1]);
        sys_sleep(1);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        display_usage();
        return -1;
    }

    char id = argv[1][0];
    if (id != 'a' && id != 'b' && id != 'c') {
        display_usage();
        return -1;
    }
    srand(clock());

    fill_mailbox_id(id);

    if (id == 'a') {
        fill_buffer();
    }

    pthread_t recv;
    pthread_create(&recv, recv_thread, (void*)(unsigned long)id);

    // use this thread as send thread
    send_thread((void*)(unsigned long)id);

    pthread_join(recv);
    return 0;
}
