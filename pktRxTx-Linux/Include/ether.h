#ifndef __ETHER_H__
#define __ETHER_H__

#include <stdint.h>

#define ETH_ALEN    6       // length of mac address

#define ETH_P_ALL   0x0003  // every packet, only used when tending to receive all packets
#define ETH_P_IP    0x0800  // IP packet 

struct ethhdr {
    uint8_t ether_dhost[ETH_ALEN];  // destination mac address
    uint8_t ether_shost[ETH_ALEN];  // source mac address
    uint16_t ether_type;            // protocol format
};

#define ETHER_HDR_SIZE sizeof(struct ethhdr)

static inline struct ethhdr *packet_to_ether_hdr(const char *packet)
{
    return (struct ethhdr *)packet;
}

#endif  // !__ETHER_H__