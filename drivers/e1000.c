#include <e1000.h>
#include <type.h>
#include <os/string.h>
#include <os/time.h>
#include <assert.h>
#include <pgtable.h>

// E1000 Registers Base Pointer
volatile uint8_t *e1000;  // use virtual memory address

// E1000 Tx & Rx Descriptors
static struct e1000_tx_desc tx_desc_array[TXDESCS] __attribute__((aligned(16)));
static struct e1000_rx_desc rx_desc_array[RXDESCS] __attribute__((aligned(16)));

// E1000 Tx & Rx packet buffer
static char tx_pkt_buffer[TXDESCS][TX_PKT_SIZE];
static char rx_pkt_buffer[RXDESCS][RX_PKT_SIZE];

// Fixed Ethernet MAC Address of E1000
static const uint8_t enetaddr[6] = {0x00, 0x0a, 0x35, 0x00, 0x1e, 0x53};

/**
 * e1000_reset - Reset Tx and Rx Units; mask and clear all interrupts.
 **/
static void e1000_reset(void)
{
	/* Turn off the ethernet interface */
    e1000_write_reg(e1000, E1000_RCTL, 0);
    e1000_write_reg(e1000, E1000_TCTL, 0);

	/* Clear the transmit ring */
    e1000_write_reg(e1000, E1000_TDH, 0);
    e1000_write_reg(e1000, E1000_TDT, 0);

	/* Clear the receive ring */
    e1000_write_reg(e1000, E1000_RDH, 0);
    e1000_write_reg(e1000, E1000_RDT, 0);

	/**
     * Delay to allow any outstanding PCI transactions to complete before
	 * resetting the device
	 */
    latency(1);

	/* Clear interrupt mask to stop board from generating interrupts */
    e1000_write_reg(e1000, E1000_IMC, 0xffffffff);

    /* Clear any pending interrupt events. */
    while (0 != e1000_read_reg(e1000, E1000_ICR)) ;
}

/**
 * e1000_configure_tx - Configure 8254x Transmit Unit after Reset
 **/
static void e1000_configure_tx(void)
{
    /* TODO: [p5-task1] Initialize tx descriptors */
    for(int i=0; i<TXDESCS; i++){
        tx_desc_array[i].addr = kva2pa(tx_pkt_buffer[i]);
        tx_desc_array[i].length = 0;
        tx_desc_array[i].cso = 0;
        tx_desc_array[i].cmd = E1000_TXD_CMD_RS;
        tx_desc_array[i].status = E1000_TXD_STAT_DD;    // 初始：传输已完成
        tx_desc_array[i].css = 0;
        tx_desc_array[i].special = 0;
    }

    /* TODO: [p5-task1] Set up the Tx descriptor base address and length */
    uint64_t tx_base_addr = kva2pa(tx_desc_array);
    uint32_t tx_base_addr_hi = tx_base_addr>>32;
    uint32_t tx_base_addr_lo = tx_base_addr & 0xffffffff;
    e1000_write_reg(e1000, E1000_TDBAL, tx_base_addr_lo);
    e1000_write_reg(e1000, E1000_TDBAH, tx_base_addr_hi);
    e1000_write_reg(e1000, E1000_TDLEN, TXDESCS*sizeof(struct e1000_tx_desc));

	/* TODO: [p5-task1] Set up the HW Tx Head and Tail descriptor pointers */
    e1000_write_reg(e1000, E1000_TDH, 0);
    e1000_write_reg(e1000, E1000_TDT, 0);

    /* TODO: [p5-task1] Program the Transmit Control Register */
    e1000_write_reg(e1000, E1000_TCTL, E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_CT & 0x100 | E1000_TCTL_COLD & 0x40000);   // ct设为0x10, cold设为0x40
    local_flush_dcache();
}

/**
 * e1000_configure_rx - Configure 8254x Receive Unit after Reset
 **/
static void e1000_configure_rx(void)
{
    /* TODO: [p5-task2] Set e1000 MAC Address to RAR[0] */

    /* TODO: [p5-task2] Initialize rx descriptors */

    /* TODO: [p5-task2] Set up the Rx descriptor base address and length */

    /* TODO: [p5-task2] Set up the HW Rx Head and Tail descriptor pointers */

    /* TODO: [p5-task2] Program the Receive Control Register */

    /* TODO: [p5-task4] Enable RXDMT0 Interrupt */
}

/**
 * e1000_init - Initialize e1000 device and descriptors
 **/
void e1000_init(void)
{
    /* Reset E1000 Tx & Rx Units; mask & clear all interrupts */
    e1000_reset();

    /* Configure E1000 Tx Unit */
    e1000_configure_tx();

    /* Configure E1000 Rx Unit */
    e1000_configure_rx();
}

/**
 * e1000_transmit - Transmit packet through e1000 net device
 * @param txpacket - The buffer address of packet to be transmitted
 * @param length - Length of this packet
 * @return - Number of bytes that are transmitted successfully
 **/
int e1000_transmit(void *txpacket, int length)
{
    /* TODO: [p5-task1] Transmit one packet from txpacket */
    // 读描述符前刷新
    local_flush_dcache();
    int tail = e1000_read_reg(e1000, E1000_TDT);
    // // 不断检查传输是否已经完成
    // while((tx_desc_array[tail].status & E1000_TXD_STAT_DD) == 0){
    //     local_flush_dcache();
    // }
    // 检查传输是否已经完成
    if((tx_desc_array[tail].status & E1000_TXD_STAT_DD) == 0){
        return 0;
    }
    tx_desc_array[tail].status = 0; // DD拉低，表示该轮传输的数据还未被DMA处理
    char * buff = tx_pkt_buffer[tail];
    // 更新length到descriptor
    tx_desc_array[tail].length = length > TX_PKT_SIZE ? TX_PKT_SIZE : length;
    // 进行数据填写并刷新
    memcpy(buff, (char*)txpacket, tx_desc_array[tail].length);
    // 传输了整个包，将end of package拉高
    if(tx_desc_array[tail].length==length)
        tx_desc_array[tail].cmd |= E1000_TXD_CMD_EOP;
    // 软件完成待传输的数据帧填写，更新tail指针
    e1000_write_reg(e1000, E1000_TDT, (tail+1)%TXDESCS);
    local_flush_dcache();
    return tx_desc_array[tail].length;
}

/**
 * e1000_poll - Receive packet through e1000 net device
 * @param rxbuffer - The address of buffer to store received packet
 * @return - Length of received packet
 **/
int e1000_poll(void *rxbuffer)
{
    /* TODO: [p5-task2] Receive one packet and put it into rxbuffer */

    return 0;
}