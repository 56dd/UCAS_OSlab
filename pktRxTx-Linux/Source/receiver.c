#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <arpa/inet.h>
#include <pcap.h>
#else
#include <windows.h>
#include "pcap.h"
#endif  // !WIN32

#include "common.h"

receiver_t g_receiver;

static char s_requests[] = "Requests: ";
static char s_response[] = "Response: ";

void send_rtp_packet(uint8_t flag){
    int hdr_len = ETHER_HDR_SIZE + IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE + RTP_BASE_HDR_SIZE;
    char *new_pkt = (char *)malloc(hdr_len);
    struct iphdr *ip_hdr = packet_to_ip_hdr(new_pkt);
    ip_hdr->ihl = IP_BASE_HDR_SIZE/4;
    struct tcphdr *tcp_hdr = packet_to_tcp_hdr(new_pkt);
    tcp_hdr->off = TCP_HDR_OFFSET;
    
    // printf("hdr len: %d %d\n", IP_HDR_SIZE(ip_hdr), TCP_HDR_SIZE(tcp_hdr));
    struct rtphdr *rtp_hdr = packet_to_rtp_hdr(new_pkt + hdr_len - RTP_BASE_HDR_SIZE);
    rtp_hdr->magic = RTP_MAGIC;
    rtp_hdr->flags = flag;
    rtp_hdr->seq = htonl(g_receiver.next_seq);
    rtp_hdr->len = htons(0);

    if (0 != pcap_sendpacket(g_handle, (const u_char *)new_pkt, hdr_len))
    {
        printf("[%s] Error: Cannot send packets due to %s ...\n", \
            __FUNCTION__, pcap_geterr(g_handle));
    }

    if(flag == RTP_ACK){
        printf("Send ack, seq = %d\n", g_receiver.next_seq);
        g_receiver.ack_cnt++;
    }
    else if(flag == RTP_RSD){
        printf("Send rsd, seq = %d\n", g_receiver.next_seq);
        g_receiver.rsd_cnt++;
    }
}

static void handle_packet(const char *packet, uint32_t pkt_len)
{
    int mode = g_config.mode;

    if (MODE_RECEIVE == mode)
    {
        g_receiver.recv_cnt += 1;
    }
    
    // Check the payload by comparing the payload
    // (Here the checksum is ignored, as students are not familiar with NW)
    struct iphdr *ip_hdr = packet_to_ip_hdr(packet);
    struct tcphdr *tcp_hdr = packet_to_tcp_hdr(packet);

    int hdr_len = ETHER_HDR_SIZE + IP_HDR_SIZE(ip_hdr) \
                + TCP_HDR_SIZE(tcp_hdr);

    if (MODE_SEND_RECEIVE == mode)
    {
        // Check whether this packet comes from the sender
        if (0 == memcmp(packet + hdr_len, s_requests, strlen(s_requests)))
        {
            return;
        }

        g_receiver.recv_cnt += 1;

        if (0 == memcmp(packet + hdr_len, s_response, strlen(s_response)))
        {
            // Next check its sequence number
            uint32_t pkt_seq = ntohl(tcp_hdr->seq);
            uint32_t pl_len = ntohs(ip_hdr->tot_len) - IP_HDR_SIZE(ip_hdr) \
                - TCP_HDR_SIZE(tcp_hdr);

            if (pkt_seq < g_receiver.next_seq)  // Duplicate Packet
            {
                g_receiver.dupl_cnt += 1;
            }
            else  // Normal or Out of order
            {
                // Get its index of the packet stream
                uint32_t pkt_idx = atoi(packet + hdr_len + strlen(s_response));
                printf("Info: Index of received packet is %u\n", pkt_idx);

                insert_rcv_ofo_buf(pkt_seq, pkt_idx, pl_len);
            }
        }
    }
    else if (MODE_RECEIVE_SEND == mode)  // Echo it back
    {
        // Check whether this packet comes from the receiver itself
        if (0 == memcmp(packet + hdr_len, s_response, strlen(s_response)))
        {
            return;
        }

        g_receiver.recv_cnt += 1;

        // Create a copy of the original packet
        char *new_pkt = (char *)malloc(pkt_len);
        if (NULL == new_pkt)
        {
            printf("[%s] Error: Cannot allocate memory for new_pkt!\n", \
                __FUNCTION__);
            exit(EXIT_FAILURE);
        }

        memcpy(new_pkt, packet, pkt_len);

        // Substitute the content of the payload
        memcpy(new_pkt + hdr_len, s_response, strlen(s_response));

        // Echo this packet back to the opposite
        if (0 != pcap_sendpacket(g_handle, (const u_char *)new_pkt, pkt_len))
        {
            printf("[%s] Error: Cannot send packets due to %s ...\n", \
                __FUNCTION__, pcap_geterr(g_handle));
        }
        else
        {
            g_receiver.echo_cnt += 1;
        }

        free(new_pkt);
    }
    else if (MODE_RELIABLE_SEND == mode)  // ACK or RESEND
    {
        struct rtphdr *rtp_hdr = packet_to_rtp_hdr(packet + hdr_len);
        if(rtp_hdr->magic != RTP_MAGIC){
            return;
        }

        g_receiver.recv_cnt += 1;

        if(rtp_hdr->flags & RTP_ACK){
            g_receiver.ack_seq = ntohl(rtp_hdr->seq);
            ack_sent_packet_buf(g_receiver.ack_seq);
            g_receiver.ack_cnt += 1;
            printf("Info: Recv ACK of seq=%d\n", g_receiver.ack_seq);
            if(g_receiver.ack_seq == g_config.file_size+4){
                printf("Info: File last ack received!\nExiting...\n");
                print_results();
                exit(0);
            }
        }
        else if(rtp_hdr->flags & RTP_RSD){
            char *payload = query_sent_packet_buf(ntohl(rtp_hdr->seq));
            if(!payload){
                g_receiver.wrong_rsd_cnt += 1;
                return;
            }
            
            struct rtphdr *resend_rtp_hdr = packet_to_rtp_hdr(payload);
            int len = ntohs(resend_rtp_hdr->len) + RTP_BASE_HDR_SIZE;
            int interval = (g_config.interval > 0) ? g_config.interval:INTERVAL_SEND;
            send_packet_one_epoch(payload, interval, len);

            g_receiver.rsd_cnt += 1;
        }
    }
    else if (MODE_RELIABLE_RECV == mode)  // send RSD & ACK
    {
        struct rtphdr *rtp_hdr = packet_to_rtp_hdr(packet + hdr_len);
        char *rtp_payload = packet_to_rtp_payload(packet + hdr_len);
        if(rtp_hdr->magic != RTP_MAGIC){
            return;
        }

        g_receiver.recv_cnt += 1;

        uint32_t pkt_seq = ntohl(rtp_hdr->seq);
        uint32_t pl_len = ntohs(rtp_hdr->len);
        if(pkt_seq == 0){
            g_config.file_size = *(uint32_t*)rtp_payload;
        }

        if (pkt_seq < g_receiver.next_seq)  // Duplicate Packet
        {
            g_receiver.dupl_cnt += 1;
        }
        else
        {
            rtp_payload[pl_len] = '\0';
            printf("Info: Received packet of seq=%u, \tlen=%u\n", pkt_seq, pl_len);
            // printf("%s\n", rtp_payload);

            uint32_t old_seq = g_receiver.next_seq;
            insert_rcv_ofo_buf(pkt_seq, 0, pl_len);

            if(old_seq != g_receiver.next_seq){
                send_rtp_packet(RTP_ACK);
                if(g_config.file_size > 0 && g_config.file_size == g_receiver.next_seq){
                    printf("Info: File receive complete!\n");
                    print_results();
                    exit(0);
                }
            }

            if(g_receiver.ack_seq < pkt_seq+pl_len){
                g_receiver.ack_seq = pkt_seq+pl_len;
            }
        }
    }

    if (0 == g_receiver.recv_cnt % g_config.rcv_pkt_interval)
    {
        print_results();
    }
}

#ifndef WIN32
void *receiver_thread(void *param)
#else
unsigned int WINAPI receiver_thread(void *param)
#endif  // !WIN32
{
    // Check whether the mode is to launch receiver
    if (0 == is_receiver_timer_mode(g_config.mode))
    {
        printf("[%s] Error: Cannot create receiver thread under mode %d\n", \
            __FUNCTION__, g_config.mode);
        exit(EXIT_FAILURE);
    }

    // Initialize the receiver
    g_receiver.recv_cnt = 0;
    g_receiver.dupl_cnt = 0;
    g_receiver.echo_cnt = 0;
    g_receiver.ack_cnt = 0;
    g_receiver.rsd_cnt = 0;
    g_receiver.wrong_rsd_cnt;
    g_receiver.next_seq = 0;
    g_receiver.next_idx = 0;
    g_receiver.ack_seq  = 0;
    init_rcv_ofo_buf();

    // Start receiving packets
    struct pcap_pkthdr *pkthdr = NULL;
    const u_char *pkt_data = NULL;

    while (NULL != g_handle)
    {
        // Try to capture those coming packets
        int ret_val = pcap_next_ex(g_handle, &pkthdr, &pkt_data);

        if (ret_val < 0)
        {
            printf("[%s] Error: Cannot capture packets due to %s ...\n", \
                __FUNCTION__, pcap_geterr(g_handle));
            break;
        }
        else if (0 == ret_val)  // Timeout, retry
        {
            continue;
        }

        // Handle this packet
        handle_packet((const char *)pkt_data, (uint32_t)pkthdr->caplen);

        set_timer();
    }

#ifndef WIN32
    return NULL;
#else
    CloseHandle(g_receiver.pid);
    return 0;
#endif  // !WIN32
}