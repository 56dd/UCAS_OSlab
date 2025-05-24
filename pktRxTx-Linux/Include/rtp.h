#ifndef __RTP_H__
#define __RTP_H__

#ifdef WIN32
#pragma pack(2)
#endif  // WIN32
struct rtphdr {
    uint8_t magic;
# define RTP_MAGIC	0x45
    uint8_t flags;
# define RTP_DAT	0x01
# define RTP_RSD	0x02
# define RTP_ACK	0x04
    uint16_t len;
    uint32_t seq;
#ifndef WIN32
} __attribute__((packed));
#else
};
#endif  // !WIN32

#define RTP_BASE_HDR_SIZE 8

#define RTP_DEFAULT_WINDOW 10000

static inline struct rtphdr *packet_to_rtp_hdr(const char *payload)
{
    return (struct rtphdr *)payload;
}

static inline char *packet_to_rtp_payload(const char *payload)
{
    return (struct rtphdr *)((char *)payload + RTP_BASE_HDR_SIZE);
}

#endif  // !__TCP_H__