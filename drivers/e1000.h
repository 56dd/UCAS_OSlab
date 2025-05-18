/*******************************************************************************

  Intel PRO/1000 Linux driver
  Copyright(c) 1999 - 2006 Intel Corporation.

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, see <http://www.gnu.org/licenses/>.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information:
  Linux NICS <linux.nics@intel.com>
  e1000-devel Mailing List <e1000-devel@lists.sourceforge.net>
  Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497

*******************************************************************************/
/* e1000_hw.h
 * Structures, enums, and macros for the MAC
 */

#ifndef _E1000_HW_H_
#define _E1000_HW_H_

#include <type.h>
#include <io.h>

/* NIC specific static variables go here */
#define TXDESCS 64          // Number of tx descriptors
#define RXDESCS 64          // Number of rx descriptors
#define TX_PKT_SIZE 2048
#define RX_PKT_SIZE 2048

/* E1000 I/O wrapper functions */
static inline void
e1000_write_reg(volatile void *addr, uint32_t reg, uint32_t val)
{
    writel(val, (uint8_t *)addr + reg);
}

static inline uint32_t
e1000_read_reg(volatile void *addr, uint32_t reg)
{
    return readl((uint8_t *)addr + reg);
}

static inline void
e1000_write_reg_array(volatile void *addr, uint32_t reg, uint32_t offset, uint32_t val)
{
    writel(val, (uint8_t *)addr + reg + (offset << 2));
}

static inline uint32_t
e1000_read_reg_array(volatile void *addr, uint32_t reg, uint32_t offset)
{
    return readl((uint8_t *)addr + reg + (offset << 2));
}

/* Receive Descriptor */
struct e1000_rx_desc {
	uint64_t addr;	    /* Address of the descriptor's data buffer */
	uint16_t length;	/* Length of data DMAed into data buffer */
	uint16_t csum;		/* Packet checksum */
	uint8_t  status;	/* Descriptor status */
	uint8_t  errors;	/* Descriptor Errors */
	uint16_t special;
}__attribute__((packed));

/* Receive Decriptor bit definitions */
#define E1000_RXD_STAT_DD	    0x01	/* Descriptor Done */
#define E1000_RXD_STAT_EOP	    0x02	/* End of Packet */
#define E1000_RXD_STAT_IXSM	    0x04	/* Ignore checksum */
#define E1000_RXD_STAT_VP	    0x08	/* IEEE VLAN Packet */
#define E1000_RXD_STAT_TCPCS	0x20	/* TCP xsum calculated */
#define E1000_RXD_STAT_IPCS	    0x40	/* IP xsum calculated */
#define E1000_RXD_STAT_PIF	    0x80	/* passed in-exact filter */
#define E1000_RXD_ERR_CE	    0x01	/* CRC Error */
#define E1000_RXD_ERR_SE	    0x02	/* Symbol Error */
#define E1000_RXD_ERR_SEQ	    0x04	/* Sequence Error */
#define E1000_RXD_ERR_CXE	    0x10	/* Carrier Extension Error */
#define E1000_RXD_ERR_TCPE	    0x20	/* TCP/UDP Checksum Error */
#define E1000_RXD_ERR_IPE	    0x40	/* IP Checksum Error */
#define E1000_RXD_ERR_RXE	    0x80	/* Rx Data Error */
#define E1000_RXD_SPC_VLAN_MASK 0x0FFF	/* VLAN ID is in lower 12 bits */
#define E1000_RXD_SPC_PRI_MASK	0xE000	/* Priority is in upper 3 bits */
#define E1000_RXD_SPC_PRI_SHIFT 0x000D	/* Priority is in upper 3 of 16 */
#define E1000_RXD_SPC_CFI_MASK	0x1000	/* CFI is bit 12 */
#define E1000_RXD_SPC_CFI_SHIFT 0x000C	/* CFI is bit 12 */

/* Transmit Descriptor */
struct e1000_tx_desc {
	uint64_t addr;	    /* Address of the descriptor's data buffer */
    uint16_t length;	/* Data buffer length */
    uint8_t cso;	    /* Checksum offset */
    uint8_t cmd;	    /* Descriptor control */
    uint8_t status;	    /* Descriptor status */
    uint8_t css;	    /* Checksum start */
    uint16_t special;
}__attribute__((packed));

/* Transmit Descriptor bit definitions */
#define E1000_TXD_DTYP_D     0x00100000	/* Data Descriptor */
#define E1000_TXD_DTYP_C     0x00000000	/* Context Descriptor */
#define E1000_TXD_POPTS_IXSM 0x01	    /* Insert IP checksum */
#define E1000_TXD_POPTS_TXSM 0x02	    /* Insert TCP/UDP checksum */
#define E1000_TXD_CMD_EOP    0x00000001	/* End of Packet */
#define E1000_TXD_CMD_IFCS   0x00000002	/* Insert FCS (Ethernet CRC) */
#define E1000_TXD_CMD_IC     0x00000004	/* Insert Checksum */
#define E1000_TXD_CMD_RS     0x00000008	/* Report Status */
#define E1000_TXD_CMD_RPS    0x00000010	/* Report Packet Sent */
#define E1000_TXD_CMD_DEXT   0x00000020	/* Descriptor extension (0 = legacy) */
#define E1000_TXD_CMD_VLE    0x00000040	/* Add VLAN tag */
#define E1000_TXD_CMD_IDE    0x00000080	/* Enable Tidv register */
#define E1000_TXD_STAT_DD    0x00000001	/* Descriptor Done */
#define E1000_TXD_STAT_EC    0x00000002	/* Excess Collisions */
#define E1000_TXD_STAT_LC    0x00000004	/* Late Collisions */
#define E1000_TXD_STAT_TU    0x00000008	/* Transmit underrun */

/* Register Set. (82543, 82544)
 *
 * Registers are defined to be 32 bits and  should be accessed as 32 bit values.
 * These registers are physically located on the NIC, but are mapped into the
 * host memory address space.
 *
 * RW - register is both readable and writable
 * RO - register is read only
 * WO - register is write only
 * R/clr - register is read only and is cleared when read
 * A - register array
 */
#define E1000_CTRL              0x00000	/* Device Control - RW */
#define E1000_STATUS            0x00008	/* Device Status - RO */
#define E1000_EECD              0x00010	/* EEPROM/Flash Control - RW */
#define E1000_I210_EECD         0x12010	/* EEPROM/Flash Control - RW */
#define E1000_EERD              0x00014	/* EEPROM Read - RW */
#define E1000_I210_EERD         0x12014	/* EEPROM Read - RW */
#define E1000_CTRL_EXT          0x00018	/* Extended Device Control - RW */
#define E1000_MDIC              0x00020	/* MDI Control - RW */
#define E1000_FCAL              0x00028	/* Flow Control Address Low - RW */
#define E1000_FCAH              0x0002C	/* Flow Control Address High -RW */
#define E1000_FCT               0x00030	/* Flow Control Type - RW */
#define E1000_VET               0x00038	/* VLAN Ether Type - RW */
#define E1000_ICR               0x000C0	/* Interrupt Cause Read - R/clr */
#define E1000_ITR               0x000C4	/* Interrupt Throttling Rate - RW */
#define E1000_ICS               0x000C8	/* Interrupt Cause Set - WO */
#define E1000_IMS               0x000D0	/* Interrupt Mask Set - RW */
#define E1000_IMC               0x000D8	/* Interrupt Mask Clear - WO */
#define E1000_I210_IAM          0x000E0	/* Interrupt Ack Auto Mask - RW */
#define E1000_RCTL              0x00100	/* RX Control - RW */
#define E1000_FCTTV             0x00170	/* Flow Control Transmit Timer Value - RW */
#define E1000_TXCW              0x00178	/* TX Configuration Word - RW */
#define E1000_RXCW              0x00180	/* RX Configuration Word - RO */
#define E1000_TCTL              0x00400	/* TX Control - RW */
#define E1000_TCTL_EXT          0x00404 /* Extended TX Control - RW */
#define E1000_TIPG              0x00410	/* TX Inter-packet gap -RW */
#define E1000_TBT               0x00448	/* TX Burst Timer - RW */
#define E1000_AIT               0x00458	/* Adaptive Interframe Spacing Throttle - RW */
#define E1000_LEDCTL            0x00E00	/* LED Control - RW */
#define E1000_EXTCNF_CTRL       0x00F00 /* Extended Configuration Control */
#define E1000_EXTCNF_SIZE       0x00F08 /* Extended Configuration Size */
#define E1000_PHY_CTRL          0x00F10 /* PHY Control Register in CSR */
#define E1000_I210_PHY_CTRL     0x00E14 /* PHY Control Register in CSR */
#define FEXTNVM_SW_CONFIG       0x0001
#define E1000_PBA               0x01000	/* Packet Buffer Allocation - RW */
#define E1000_PBS               0x01008  /* Packet Buffer Size */
#define E1000_EEMNGCTL          0x01010  /* MNG EEprom Control */
#define E1000_I210_EEMNGCTL     0x12030  /* MNG EEprom Control */
#define E1000_FLASH_UPDATES     1000
#define E1000_EEARBC            0x01024  /* EEPROM Auto Read Bus Control */
#define E1000_FLASHT            0x01028  /* FLASH Timer Register */
#define E1000_EEWR              0x0102C  /* EEPROM Write Register - RW */
#define E1000_I210_EEWR         0x12018  /* EEPROM Write Register - RW */
#define E1000_FLSWCTL           0x01030  /* FLASH control register */
#define E1000_FLSWDATA          0x01034  /* FLASH data register */
#define E1000_FLSWCNT           0x01038  /* FLASH Access Counter */
#define E1000_FLOP              0x0103C  /* FLASH Opcode Register */
#define E1000_ERT               0x02008  /* Early Rx Threshold - RW */
#define E1000_FCRTL             0x02160	/* Flow Control Receive Threshold Low - RW */
#define E1000_FCRTH             0x02168	/* Flow Control Receive Threshold High - RW */
#define E1000_RDBAL             0x02800	/* RX Descriptor Base Address Low - RW */
#define E1000_RDBAH             0x02804	/* RX Descriptor Base Address High - RW */
#define E1000_RDLEN             0x02808	/* RX Descriptor Length - RW */
#define E1000_RDH               0x02810	/* RX Descriptor Head - RW */
#define E1000_RDT               0x02818	/* RX Descriptor Tail - RW */
#define E1000_RDTR              0x02820	/* RX Delay Timer - RW */
#define E1000_RXDCTL            0x02828	/* RX Descriptor Control - RW */
#define E1000_RADV              0x0282C	/* RX Interrupt Absolute Delay Timer - RW */
#define E1000_RSRPD             0x02C00	/* RX Small Packet Detect - RW */
#define E1000_TXDMAC            0x03000	/* TX DMA Control - RW */
#define E1000_TDFH              0x03410  /* TX Data FIFO Head - RW */
#define E1000_TDFT              0x03418  /* TX Data FIFO Tail - RW */
#define E1000_TDFHS             0x03420  /* TX Data FIFO Head Saved - RW */
#define E1000_TDFTS             0x03428  /* TX Data FIFO Tail Saved - RW */
#define E1000_TDFPC             0x03430  /* TX Data FIFO Packet Count - RW */
#define E1000_TDBAL             0x03800	/* TX Descriptor Base Address Low - RW */
#define E1000_TDBAH             0x03804	/* TX Descriptor Base Address High - RW */
#define E1000_TDLEN             0x03808	/* TX Descriptor Length - RW */
#define E1000_TDH               0x03810	/* TX Descriptor Head - RW */
#define E1000_TDT               0x03818	/* TX Descripotr Tail - RW */
#define E1000_TIDV              0x03820	/* TX Interrupt Delay Value - RW */
#define E1000_TXDCTL            0x03828	/* TX Descriptor Control - RW */
#define E1000_TADV              0x0382C	/* TX Interrupt Absolute Delay Val - RW */
#define E1000_TSPMT             0x03830	/* TCP Segmentation PAD & Min Threshold - RW */
#define E1000_TARC0             0x03840  /* TX Arbitration Count (0) */
#define E1000_TDBAL1            0x03900  /* TX Desc Base Address Low (1) - RW */
#define E1000_TDBAH1            0x03904  /* TX Desc Base Address High (1) - RW */
#define E1000_TDLEN1            0x03908  /* TX Desc Length (1) - RW */
#define E1000_TDH1              0x03910  /* TX Desc Head (1) - RW */
#define E1000_TDT1              0x03918  /* TX Desc Tail (1) - RW */
#define E1000_TXDCTL1           0x03928  /* TX Descriptor Control (1) - RW */
#define E1000_TARC1             0x03940  /* TX Arbitration Count (1) */
#define E1000_CRCERRS           0x04000	/* CRC Error Count - R/clr */
#define E1000_ALGNERRC          0x04004	/* Alignment Error Count - R/clr */
#define E1000_SYMERRS           0x04008	/* Symbol Error Count - R/clr */
#define E1000_RXERRC            0x0400C	/* Receive Error Count - R/clr */
#define E1000_MPC               0x04010	/* Missed Packet Count - R/clr */
#define E1000_SCC               0x04014	/* Single Collision Count - R/clr */
#define E1000_ECOL              0x04018	/* Excessive Collision Count - R/clr */
#define E1000_MCC               0x0401C	/* Multiple Collision Count - R/clr */
#define E1000_LATECOL           0x04020	/* Late Collision Count - R/clr */
#define E1000_COLC              0x04028	/* Collision Count - R/clr */
#define E1000_DC                0x04030	/* Defer Count - R/clr */
#define E1000_TNCRS             0x04034	/* TX-No CRS - R/clr */
#define E1000_SEC               0x04038	/* Sequence Error Count - R/clr */
#define E1000_CEXTERR           0x0403C	/* Carrier Extension Error Count - R/clr */
#define E1000_RLEC              0x04040	/* Receive Length Error Count - R/clr */
#define E1000_XONRXC            0x04048	/* XON RX Count - R/clr */
#define E1000_XONTXC            0x0404C	/* XON TX Count - R/clr */
#define E1000_XOFFRXC           0x04050	/* XOFF RX Count - R/clr */
#define E1000_XOFFTXC           0x04054	/* XOFF TX Count - R/clr */
#define E1000_FCRUC             0x04058	/* Flow Control RX Unsupported Count- R/clr */
#define E1000_PRC64             0x0405C	/* Packets RX (64 bytes) - R/clr */
#define E1000_PRC127            0x04060	/* Packets RX (65-127 bytes) - R/clr */
#define E1000_PRC255            0x04064	/* Packets RX (128-255 bytes) - R/clr */
#define E1000_PRC511            0x04068	/* Packets RX (255-511 bytes) - R/clr */
#define E1000_PRC1023           0x0406C	/* Packets RX (512-1023 bytes) - R/clr */
#define E1000_PRC1522           0x04070	/* Packets RX (1024-1522 bytes) - R/clr */
#define E1000_GPRC              0x04074	/* Good Packets RX Count - R/clr */
#define E1000_BPRC              0x04078	/* Broadcast Packets RX Count - R/clr */
#define E1000_MPRC              0x0407C	/* Multicast Packets RX Count - R/clr */
#define E1000_GPTC              0x04080	/* Good Packets TX Count - R/clr */
#define E1000_GORCL             0x04088	/* Good Octets RX Count Low - R/clr */
#define E1000_GORCH             0x0408C	/* Good Octets RX Count High - R/clr */
#define E1000_GOTCL             0x04090	/* Good Octets TX Count Low - R/clr */
#define E1000_GOTCH             0x04094	/* Good Octets TX Count High - R/clr */
#define E1000_RNBC              0x040A0	/* RX No Buffers Count - R/clr */
#define E1000_RUC               0x040A4	/* RX Undersize Count - R/clr */
#define E1000_RFC               0x040A8	/* RX Fragment Count - R/clr */
#define E1000_ROC               0x040AC	/* RX Oversize Count - R/clr */
#define E1000_RJC               0x040B0	/* RX Jabber Count - R/clr */
#define E1000_MGTPRC            0x040B4	/* Management Packets RX Count - R/clr */
#define E1000_MGTPDC            0x040B8	/* Management Packets Dropped Count - R/clr */
#define E1000_MGTPTC            0x040BC	/* Management Packets TX Count - R/clr */
#define E1000_TORL              0x040C0	/* Total Octets RX Low - R/clr */
#define E1000_TORH              0x040C4	/* Total Octets RX High - R/clr */
#define E1000_TOTL              0x040C8	/* Total Octets TX Low - R/clr */
#define E1000_TOTH              0x040CC	/* Total Octets TX High - R/clr */
#define E1000_TPR               0x040D0	/* Total Packets RX - R/clr */
#define E1000_TPT               0x040D4	/* Total Packets TX - R/clr */
#define E1000_PTC64             0x040D8	/* Packets TX (64 bytes) - R/clr */
#define E1000_PTC127            0x040DC	/* Packets TX (65-127 bytes) - R/clr */
#define E1000_PTC255            0x040E0	/* Packets TX (128-255 bytes) - R/clr */
#define E1000_PTC511            0x040E4	/* Packets TX (256-511 bytes) - R/clr */
#define E1000_PTC1023           0x040E8	/* Packets TX (512-1023 bytes) - R/clr */
#define E1000_PTC1522           0x040EC	/* Packets TX (1024-1522 Bytes) - R/clr */
#define E1000_MPTC              0x040F0	/* Multicast Packets TX Count - R/clr */
#define E1000_BPTC              0x040F4	/* Broadcast Packets TX Count - R/clr */
#define E1000_TSCTC             0x040F8	/* TCP Segmentation Context TX - R/clr */
#define E1000_TSCTFC            0x040FC	/* TCP Segmentation Context TX Fail - R/clr */
#define E1000_RXCSUM            0x05000	/* RX Checksum Control - RW */
#define E1000_MTA               0x05200	/* Multicast Table Array - RW Array */
#define E1000_RA                0x05400	/* Receive Address - RW Array */
#define E1000_VFTA              0x05600	/* VLAN Filter Table Array - RW Array */
#define E1000_WUC               0x05800	/* Wakeup Control - RW */
#define E1000_WUFC              0x05808	/* Wakeup Filter Control - RW */
#define E1000_WUS               0x05810	/* Wakeup Status - RO */
#define E1000_MANC              0x05820	/* Management Control - RW */
#define E1000_IPAV              0x05838	/* IP Address Valid - RW */
#define E1000_IP4AT             0x05840	/* IPv4 Address Table - RW Array */
#define E1000_IP6AT             0x05880	/* IPv6 Address Table - RW Array */
#define E1000_WUPL              0x05900	/* Wakeup Packet Length - RW */
#define E1000_WUPM              0x05A00	/* Wakeup Packet Memory - RO A */
#define E1000_FFLT              0x05F00	/* Flexible Filter Length Table - RW Array */
#define E1000_FFMT              0x09000	/* Flexible Filter Mask Table - RW Array */
#define E1000_FFVT              0x09800	/* Flexible Filter Value Table - RW Array */

/* Receive Address */
#define E1000_RAH_AV            0x80000000	/* Receive descriptor valid */

/* Interrupt Cause Read */
#define E1000_ICR_TXDW	  0x00000001	/* Transmit desc written back */
#define E1000_ICR_TXQE	  0x00000002	/* Transmit Queue empty */
#define E1000_ICR_LSC	  0x00000004	/* Link Status Change */
#define E1000_ICR_RXSEQ   0x00000008	/* rx sequence error */
#define E1000_ICR_RXDMT0  0x00000010	/* rx desc min. threshold (0) */
#define E1000_ICR_RXO	  0x00000040	/* rx overrun */
#define E1000_ICR_RXT0	  0x00000080	/* rx timer intr (ring 0) */
#define E1000_ICR_MDAC	  0x00000200	/* MDIO access complete */
#define E1000_ICR_RXCFG   0x00000400	/* RX /c/ ordered set */
#define E1000_ICR_GPI_EN0 0x00000800	/* GP Int 0 */
#define E1000_ICR_GPI_EN1 0x00001000	/* GP Int 1 */
#define E1000_ICR_GPI_EN2 0x00002000	/* GP Int 2 */
#define E1000_ICR_GPI_EN3 0x00004000	/* GP Int 3 */
#define E1000_ICR_TXD_LOW 0x00008000
#define E1000_ICR_SRPD	  0x00010000

/* Interrupt Cause Set */
#define E1000_ICS_TXDW	  E1000_ICR_TXDW	/* Transmit desc written back */
#define E1000_ICS_TXQE	  E1000_ICR_TXQE	/* Transmit Queue empty */
#define E1000_ICS_LSC	  E1000_ICR_LSC	    /* Link Status Change */
#define E1000_ICS_RXSEQ   E1000_ICR_RXSEQ	/* rx sequence error */
#define E1000_ICS_RXDMT0  E1000_ICR_RXDMT0	/* rx desc min. threshold */
#define E1000_ICS_RXO	  E1000_ICR_RXO	    /* rx overrun */
#define E1000_ICS_RXT0	  E1000_ICR_RXT0	/* rx timer intr */
#define E1000_ICS_MDAC	  E1000_ICR_MDAC	/* MDIO access complete */
#define E1000_ICS_RXCFG   E1000_ICR_RXCFG	/* RX /c/ ordered set */
#define E1000_ICS_GPI_EN0 E1000_ICR_GPI_EN0	/* GP Int 0 */
#define E1000_ICS_GPI_EN1 E1000_ICR_GPI_EN1	/* GP Int 1 */
#define E1000_ICS_GPI_EN2 E1000_ICR_GPI_EN2	/* GP Int 2 */
#define E1000_ICS_GPI_EN3 E1000_ICR_GPI_EN3	/* GP Int 3 */
#define E1000_ICS_TXD_LOW E1000_ICR_TXD_LOW
#define E1000_ICS_SRPD	  E1000_ICR_SRPD

/* Interrupt Mask Set */
#define E1000_IMS_TXDW	  E1000_ICR_TXDW	/* Transmit desc written back */
#define E1000_IMS_TXQE	  E1000_ICR_TXQE	/* Transmit Queue empty */
#define E1000_IMS_LSC	  E1000_ICR_LSC	    /* Link Status Change */
#define E1000_IMS_RXSEQ   E1000_ICR_RXSEQ	/* rx sequence error */
#define E1000_IMS_RXDMT0  E1000_ICR_RXDMT0	/* rx desc min. threshold */
#define E1000_IMS_RXO	  E1000_ICR_RXO	    /* rx overrun */
#define E1000_IMS_RXT0	  E1000_ICR_RXT0	/* rx timer intr */
#define E1000_IMS_MDAC	  E1000_ICR_MDAC	/* MDIO access complete */
#define E1000_IMS_RXCFG   E1000_ICR_RXCFG	/* RX /c/ ordered set */
#define E1000_IMS_GPI_EN0 E1000_ICR_GPI_EN0	/* GP Int 0 */
#define E1000_IMS_GPI_EN1 E1000_ICR_GPI_EN1	/* GP Int 1 */
#define E1000_IMS_GPI_EN2 E1000_ICR_GPI_EN2	/* GP Int 2 */
#define E1000_IMS_GPI_EN3 E1000_ICR_GPI_EN3	/* GP Int 3 */
#define E1000_IMS_TXD_LOW E1000_ICR_TXD_LOW
#define E1000_IMS_SRPD	  E1000_ICR_SRPD

/* Interrupt Mask Clear */
#define E1000_IMC_TXDW	  E1000_ICR_TXDW	/* Transmit desc written back */
#define E1000_IMC_TXQE	  E1000_ICR_TXQE	/* Transmit Queue empty */
#define E1000_IMC_LSC	  E1000_ICR_LSC	/* Link Status Change */
#define E1000_IMC_RXSEQ   E1000_ICR_RXSEQ	/* rx sequence error */
#define E1000_IMC_RXDMT0  E1000_ICR_RXDMT0	/* rx desc min. threshold */
#define E1000_IMC_RXO	  E1000_ICR_RXO	/* rx overrun */
#define E1000_IMC_RXT0	  E1000_ICR_RXT0	/* rx timer intr */
#define E1000_IMC_MDAC	  E1000_ICR_MDAC	/* MDIO access complete */
#define E1000_IMC_RXCFG   E1000_ICR_RXCFG	/* RX /c/ ordered set */
#define E1000_IMC_GPI_EN0 E1000_ICR_GPI_EN0	/* GP Int 0 */
#define E1000_IMC_GPI_EN1 E1000_ICR_GPI_EN1	/* GP Int 1 */
#define E1000_IMC_GPI_EN2 E1000_ICR_GPI_EN2	/* GP Int 2 */
#define E1000_IMC_GPI_EN3 E1000_ICR_GPI_EN3	/* GP Int 3 */
#define E1000_IMC_TXD_LOW E1000_ICR_TXD_LOW
#define E1000_IMC_SRPD	  E1000_ICR_SRPD

/* Receive Control */
#define E1000_RCTL_RST		    0x00000001	/* Software reset */
#define E1000_RCTL_EN		    0x00000002	/* enable */
#define E1000_RCTL_SBP		    0x00000004	/* store bad packet */
#define E1000_RCTL_UPE		    0x00000008	/* unicast promiscuous enable */
#define E1000_RCTL_MPE		    0x00000010	/* multicast promiscuous enab */
#define E1000_RCTL_LPE		    0x00000020	/* long packet enable */
#define E1000_RCTL_LBM_NO	    0x00000000	/* no loopback mode */
#define E1000_RCTL_LBM_MAC	    0x00000040	/* MAC loopback mode */
#define E1000_RCTL_LBM_SLP	    0x00000080	/* serial link loopback mode */
#define E1000_RCTL_LBM_TCVR	    0x000000C0	/* tcvr loopback mode */
#define E1000_RCTL_RDMTS_HALF	0x00000000	/* rx desc min threshold size */
#define E1000_RCTL_RDMTS_QUAT	0x00000100	/* rx desc min threshold size */
#define E1000_RCTL_RDMTS_EIGTH	0x00000200	/* rx desc min threshold size */
#define E1000_RCTL_MO_SHIFT	12	/* multicast offset shift */
#define E1000_RCTL_MO_0		    0x00000000	/* multicast offset 11:0 */
#define E1000_RCTL_MO_1		    0x00001000	/* multicast offset 12:1 */
#define E1000_RCTL_MO_2		    0x00002000	/* multicast offset 13:2 */
#define E1000_RCTL_MO_3		    0x00003000	/* multicast offset 15:4 */
#define E1000_RCTL_MDR		    0x00004000	/* multicast desc ring 0 */
#define E1000_RCTL_BAM		    0x00008000	/* broadcast enable */
/* these buffer sizes are valid if E1000_RCTL_BSEX is 0 */
#define E1000_RCTL_SZ_2048	    0x00000000	/* rx buffer size 2048 */
#define E1000_RCTL_SZ_1024	    0x00010000	/* rx buffer size 1024 */
#define E1000_RCTL_SZ_512	    0x00020000	/* rx buffer size 512 */
#define E1000_RCTL_SZ_256	    0x00030000	/* rx buffer size 256 */
/* these buffer sizes are valid if E1000_RCTL_BSEX is 1 */
#define E1000_RCTL_SZ_16384	    0x00010000	/* rx buffer size 16384 */
#define E1000_RCTL_SZ_8192	    0x00020000	/* rx buffer size 8192 */
#define E1000_RCTL_SZ_4096	    0x00030000	/* rx buffer size 4096 */
#define E1000_RCTL_VFE		    0x00040000	/* vlan filter enable */
#define E1000_RCTL_CFIEN	    0x00080000	/* canonical form enable */
#define E1000_RCTL_CFI		    0x00100000	/* canonical form indicator */
#define E1000_RCTL_DPF		    0x00400000	/* discard pause frames */
#define E1000_RCTL_PMCF		    0x00800000	/* pass MAC control frames */
#define E1000_RCTL_BSEX		    0x02000000	/* Buffer size extension */

/* Transmit Control */
#define E1000_TCTL_RST	  0x00000001	/* software reset */
#define E1000_TCTL_EN	  0x00000002	/* enable tx */
#define E1000_TCTL_BCE	  0x00000004	/* busy check enable */
#define E1000_TCTL_PSP	  0x00000008	/* pad short packets */
#define E1000_TCTL_CT	  0x00000ff0	/* collision threshold */
#define E1000_TCTL_COLD   0x003ff000	/* collision distance */
#define E1000_TCTL_SWXOFF 0x00400000	/* SW Xoff transmission */
#define E1000_TCTL_PBE	  0x00800000	/* Packet Burst Enable */
#define E1000_TCTL_RTLC   0x01000000	/* Re-transmit on late collision */
#define E1000_TCTL_NRTU   0x02000000	/* No Re-transmit on underrun */
#define E1000_TCTL_MULR   0x10000000    /* Multiple request support */

/* Default values for the transmit IPG register */
#define DEFAULT_82542_TIPG_IPGT        10
#define DEFAULT_82543_TIPG_IPGT_FIBER  9
#define DEFAULT_82543_TIPG_IPGT_COPPER 8

#define E1000_TIPG_IPGT_MASK  0x000003FF
#define E1000_TIPG_IPGR1_MASK 0x000FFC00
#define E1000_TIPG_IPGR2_MASK 0x3FF00000

#define DEFAULT_82542_TIPG_IPGR1 2
#define DEFAULT_82543_TIPG_IPGR1 8
#define E1000_TIPG_IPGR1_SHIFT	10

#define DEFAULT_82542_TIPG_IPGR2 10
#define DEFAULT_82543_TIPG_IPGR2 6
#define DEFAULT_80003ES2LAN_TIPG_IPGR2 7
#define E1000_TIPG_IPGR2_SHIFT	20

/* E1000 Base Address Pointer */
extern volatile uint8_t *e1000;

/* E1000 Function Definitions */
void e1000_init(void);
int e1000_transmit(void *txpacket, int length);
int e1000_poll(void *rxbuffer);

#endif  // !_E1000_HW_H_