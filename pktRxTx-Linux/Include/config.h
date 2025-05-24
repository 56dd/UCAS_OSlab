#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "protocol.h"

#define MAX_DEV_NAME_LEN	 1000
#define MAX_PATH_LEN	     1000
#define MAX_MODES            7

#define MODE_INVALID         0  // Initial mode, to judge if it's enable
#define MODE_SEND            1  // Send pkts to the peer
#define MODE_SEND_RECEIVE    2  // Send pkts, while receiving them echoed from the peer
#define MODE_RECEIVE         3  // Receives pkts from the peer, not check their validity
#define MODE_RECEIVE_SEND    4  // Receive pkts, and echo them back to the peer
#define MODE_RELIABLE_SEND   5  // Send pkts with a reliable transport layer protocal
#define MODE_RELIABLE_RECV   6  // Recv pkts with a reliable transport layer protocal

#define DEFAULT_PKT_INTERVAL 1  // Print results when receiving 1 new packet

typedef struct {
    int mode;                   // Which mode to use
    int num_pkts;               // How many packets to send
    int interval;               // Interval between two packets to be sent
    int rcv_pkt_interval;       // Interval between print_results is called in receiver
    int window;                 // Size of sending window in SEND_RELIABLE mode
    char filename[MAX_PATH_LEN];
    uint32_t file_size;
    int loss;
    int shuffle;

    char dev[MAX_DEV_NAME_LEN]; // Name of this device

    uint8_t src_mac[ETH_ALEN];  // Source MAC Address
    uint8_t dst_mac[ETH_ALEN];  // Destination MAC Address

    uint32_t saddr;             // Source IP Address
    uint32_t daddr;             // Destination IP Address

    uint16_t sport;             // Source port
    uint16_t dport1;            // Destination port1, set for 2021-UCAS-OS
    uint16_t dport2;            // Destination port2, set for 2021-UCAS-OS
} config_t;

extern config_t g_config;

void load_config(int argc, char *argv[]);

static inline int is_sender_mode(int mode)
{
    // Under this mode, the sender should be created
    return (int)(MODE_SEND == mode || MODE_SEND_RECEIVE == mode \
                                   || MODE_RELIABLE_SEND == mode);
}

static inline int is_receiver_timer_mode(int mode)
{
    // Under this mode, the receiver and timer should be created
    return (int)(MODE_RECEIVE == mode || MODE_SEND_RECEIVE == mode \
                                      || MODE_RECEIVE_SEND == mode \
                                      || MODE_RELIABLE_SEND == mode \
                                      || MODE_RELIABLE_RECV == mode);
}

#endif  // !__CONFIG_H__
