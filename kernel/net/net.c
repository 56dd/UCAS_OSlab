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
    int total_bytes = 0;
    for(int i=0; i<pkt_num;){
        pkt_lens[i]= e1000_poll(rxbuffer);
        //if(pkt_lens[i]==0){
        //    do_block(&current_running[cpu_id]->list, &recv_block_queue);
        //    continue;   // 暂时不能更新index，上次未成功收包
        //}
        rxbuffer += pkt_lens[i];
        total_bytes += pkt_lens[i];
        i++;
    }
    
    return total_bytes;  // Bytes it has received
}


void net_handle_irq(void)
{
    // TODO: [p5-task4] Handle interrupts from network device
    local_flush_dcache();
    uint32_t cause = e1000_read_reg(e1000, E1000_ICR);
    list_node_t *tmp;
    // 检查接收阻塞队列是否需要释放进程
    int rdt = (e1000_read_reg(e1000, E1000_RDT)+1)%RXDESCS;
    if((tmp = recv_block_queue.next) != &recv_block_queue && (rx_desc_array[rdt].status & E1000_RXD_STAT_DD))
        do_unblock(tmp);
    // 发送描述符不够
    if(cause==E1000_ICR_TXQE){
        // 关闭TXQE中断
        e1000_write_reg(e1000, E1000_IMC, E1000_IMC_TXQE);
        do_block(&current_running[cpu_id]->list, &send_block_queue);
    }
    // 硬件接收描述符不够，需要唤醒接收进程，顺便检查发送阻塞队列
    else if(cause==E1000_ICR_RXDMT0){
        // 检查发送阻塞队列是否需要释放进程
        int tdt = e1000_read_reg(e1000, E1000_TDT);
        // 队列非空且DD位拉高
        if((tmp = send_block_queue.next) != &send_block_queue && (tx_desc_array[tdt].status & E1000_TXD_STAT_DD))
            do_unblock(tmp);
    }
}