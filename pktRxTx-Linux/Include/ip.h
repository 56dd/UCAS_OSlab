#ifndef __IP_H__
#define __IP_H__

#include <stdint.h>

#include "ether.h"
#include "checksum.h"

#define DEFAULT_TTL     64
#define IP_DF	        0x4000		// do not fragment

#ifndef IPPROTO_TCP
#define IPPROTO_TCP     6
#endif  // !IPPROTO_TCP

struct iphdr {
    uint8_t ihl : 4;        // length of ip header
    uint8_t version : 4;    // ip version 
    uint8_t tos;            // type of service (usually set to 0)
    uint16_t tot_len;       // total length of ip data  
    uint16_t id;            // ip identifier
    uint16_t frag_off;      // the offset of ip fragment
    uint8_t ttl;            // ttl of ip packet
    uint8_t protocol;       // upper layer protocol, e.g. icmp, tcp, udp 
    uint16_t checksum;      // checksum of ip header
    uint32_t saddr;         // source ip address
    uint32_t daddr;         // destination ip address
};

#define IP_BASE_HDR_SIZE sizeof(struct iphdr)
#define IP_HDR_SIZE(hdr) (hdr->ihl * 4)
#define IP_DATA(hdr)    ((char *)hdr + IP_HDR_SIZE(hdr))

static inline uint16_t ip_checksum(struct iphdr *hdr)
{
    uint16_t tmp = hdr->checksum;
    hdr->checksum = 0;
    uint16_t sum = checksum((uint16_t *)hdr, hdr->ihl * 4, 0);
    hdr->checksum = tmp;

    return sum;
}

static inline struct iphdr *packet_to_ip_hdr(const char *packet)
{
    return (struct iphdr *)(packet + ETHER_HDR_SIZE);
}

#endif  // !__IP_H__