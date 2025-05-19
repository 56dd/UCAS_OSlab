#include <e1000.h>
#include <type.h>
#include <os/sched.h>
#include <os/string.h>
#include <os/list.h>
#include <os/smp.h>

static LIST_HEAD(send_block_queue);
static LIST_HEAD(recv_block_queue);

int do_net_send(void *txpacket, int length)
{
    int trans_len;
    while(1){
        trans_len = e1000_transmit(txpacket, length);
        if(trans_len==0){
            // // TODO: [p5-task3] Call do_block when e1000 transmit queue is full
            // do_block(&current_running[cpu_id]->list, &send_block_queue);
            // TODO: [p5-task4] Enable TXQE interrupt if transmit queue is full
            // 开启TXQE发送中断
            e1000_write_reg(e1000, E1000_IMS, E1000_IMS_TXQE);
        }
        else
            break;
    }
    
    return trans_len;  // Bytes it has transmitted
}

int do_net_recv(void *rxbuffer, int pkt_num, int *pkt_lens)
{
    // TODO: [p5-task2] Receive one network packet via e1000 device
    // TODO: [p5-task3] Call do_block when there is no packet on the way

    return 0;  // Bytes it has received
}

void net_handle_irq(void)
{
    // TODO: [p5-task4] Handle interrupts from network device
}