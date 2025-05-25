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
            // TODO: [p5-task3] Call do_block when e1000 transmit queue is full
            // TODO: [p5-task4] Enable TXQE interrupt if transmit queue is full
            // 开启TXQE发送中断
            e1000_write_reg(e1000, E1000_IMS, E1000_IMS_TXQE);
            local_flush_dcache();
            do_block(&current_running[cpu_id]->list, &send_block_queue);
            do_scheduler();
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
        if(pkt_lens[i]==0){
            e1000_write_reg(e1000, E1000_IMS, E1000_IMS_RXDMT0);
            local_flush_dcache();
            do_block(&current_running[cpu_id]->list, &recv_block_queue);
            do_scheduler();
            continue;   // 暂时不能更新index，上次未成功收包
        }
        rxbuffer += pkt_lens[i];
        total_bytes += pkt_lens[i];
        i++;
    }
    
    return total_bytes;  // Bytes it has received
}

static void handle_e1000_txqe(void)
{
    free_block_list(&send_block_queue);
    /* disable TXQE interrupt */
    e1000_write_reg(e1000, E1000_IMC, E1000_IMC_TXQE);
    local_flush_dcache();
}

static void handle_e1000_rxdmt0(void)
{
    free_block_list(&recv_block_queue);
    /* disable RXDMT0 interrupt */
    e1000_write_reg(e1000, E1000_IMC, E1000_IMC_RXDMT0);
    local_flush_dcache();
}

void net_handle_irq(void)
{
    // TODO: [p5-task4] Handle interrupts from network device
    local_flush_dcache();
    uint32_t icr = e1000_read_reg(e1000, E1000_ICR);
    if (icr & E1000_ICR_TXQE)
        handle_e1000_txqe();
    if (icr & E1000_ICR_RXDMT0)
        handle_e1000_rxdmt0();
}