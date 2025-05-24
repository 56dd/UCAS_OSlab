#ifndef __TCP_H__
#define __TCP_H__

#include "ip.h"
#include "checksum.h"

#ifndef WIN32
#include <arpa/inet.h>
#else
#include <windows.h>
#endif  // !WIN32

#define DEFAULT_SPORT	46930
#define DEFAULT_DPORT1  50001
#define DEFAULT_DPORT2  58688

#ifdef WIN32
#pragma pack(2)
#endif  // WIN32
struct tcphdr {
    uint16_t sport;		    // source port 
    uint16_t dport;		    // destination port
    uint32_t seq;			// sequence number
    uint32_t ack;			// acknowledgement number
    uint8_t x2 : 4;			// (unused)
    uint8_t off : 4;			// data offset
    uint8_t flags;
# define TCP_FIN	0x01
# define TCP_SYN	0x02
# define TCP_RST	0x04
# define TCP_PSH	0x08
# define TCP_ACK	0x10
# define TCP_URG	0x20
    uint16_t rwnd;			// receiving window
    uint16_t checksum;		// checksum
    uint16_t urp;			// urgent pointer
#ifndef WIN32
} __attribute__((packed));
#else
};
#endif  // !WIN32

#define TCP_HDR_OFFSET 5
#define TCP_BASE_HDR_SIZE 20
#define TCP_HDR_SIZE(tcp) (tcp->off * 4)

#define TCP_DEFAULT_WINDOW 65535

static inline struct tcphdr *packet_to_tcp_hdr(const char *packet)
{
    struct iphdr *ip = packet_to_ip_hdr(packet);
    return (struct tcphdr *)((char *)ip + IP_HDR_SIZE(ip));
}

static inline uint16_t tcp_checksum(struct iphdr *ip, struct tcphdr *tcp)
{
    uint16_t tmp = tcp->checksum;
    tcp->checksum = 0;

    uint16_t reserv_proto = ip->protocol;
    uint16_t tcp_len = ntohs(ip->tot_len) - IP_HDR_SIZE(ip);

    uint32_t sum = ip->saddr + ip->daddr + htons(reserv_proto) \
        + htons(tcp_len);
    uint16_t cksum = checksum((uint16_t *)tcp, (int)tcp_len, sum);

    tcp->checksum = tmp;

    return cksum;
}

#endif  // !__TCP_H__