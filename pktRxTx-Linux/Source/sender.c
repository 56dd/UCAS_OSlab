#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/time.h>
#else
#include <time.h>
#include <windows.h>
#include "pcap.h"
#endif  // !WIN32

#include "common.h"

sender_t g_sender;

static char s_requests[] = "Requests: ";

static uint32_t fill_payload(char *payload, int limit, uint32_t idx)
{
    char *buf = (char *)malloc(limit * sizeof(char));
    if (NULL == buf)
    {
        printf("[%s] Error: Cannot allocate more memory for payload!\n", \
                __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    char number[100] = {'\0'};
    sprintf(number, "%u", idx);

    if (MODE_SEND == g_config.mode){
        strcpy(buf, s_requests);
        strcat(buf, number);
        strcat(buf, "(^_^)");  // To ensure that atoi function works successfully

        memcpy(payload, buf, strlen(buf));

        free(buf);

        return strlen(payload);
    }
    else if (MODE_RELIABLE_SEND == g_config.mode){
        uint32_t len = limit - RTP_BASE_HDR_SIZE;
        struct rtphdr *rtp_hdr = packet_to_rtp_hdr(payload);
        char *rtp_payload = packet_to_rtp_payload(payload);
        rtp_hdr->magic = RTP_MAGIC;
        rtp_hdr->flags = RTP_DAT;
        rtp_hdr->seq   = htonl(read_send_buf(rtp_payload, &len));
        rtp_hdr->len   = htons(len);

        if(len == 0){
            free(buf);
            return 0;
        }

        memcpy(buf, payload, len + RTP_BASE_HDR_SIZE);
        insert_sent_packet_buf(ntohl(rtp_hdr->seq), buf);

        return len + RTP_BASE_HDR_SIZE;
    }
}

static int send_packet(const char *payload, uint32_t seq, uint32_t ack, uint32_t len)
{
    // Create a new packet
    int hdr_len = ETHER_HDR_SIZE + IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE;
    int pl_len = len;
    int pkt_len = hdr_len + pl_len;

    char *pkt = (char *)malloc(pkt_len);
    if (NULL == pkt)
    {
        printf("[%s] Error: Cannot allocate for a new packet!\n", \
            __FUNCTION__);
        return 0;  // Which means having sent 0 bytes successfully
    }

    memset(pkt, 0, pkt_len);

    // Assembly ether header
    struct ethhdr *eth_hdr = packet_to_ether_hdr(pkt);

    memcpy(eth_hdr->ether_dhost, g_config.dst_mac, ETH_ALEN);
    memcpy(eth_hdr->ether_shost, g_config.src_mac, ETH_ALEN);
    eth_hdr->ether_type = htons(ETH_P_IP);

    // Assembly ip header
    struct iphdr *ip_hdr = packet_to_ip_hdr(pkt);

    ip_hdr->ihl = 5;
    ip_hdr->version = 4;
    ip_hdr->tos = 0;
    ip_hdr->tot_len = htons(IP_BASE_HDR_SIZE + TCP_BASE_HDR_SIZE + pl_len);
    ip_hdr->id = htons(54321);
    ip_hdr->frag_off = htons(IP_DF);
    ip_hdr->ttl = DEFAULT_TTL;
    ip_hdr->protocol = IPPROTO_TCP;
    ip_hdr->saddr = htonl(g_config.saddr);
    ip_hdr->daddr = htonl(g_config.daddr);

    // Assembly tcp header and payload
    struct tcphdr *tcp_hdr = packet_to_tcp_hdr(pkt);

    tcp_hdr->sport = htons(g_config.sport);
    tcp_hdr->dport = htons((rand() & 0x1) ? g_config.dport1 : g_config.dport2);
    tcp_hdr->seq = htonl(seq);
    tcp_hdr->ack = htonl(ack);
    tcp_hdr->off = TCP_HDR_OFFSET;
    tcp_hdr->flags = TCP_PSH | TCP_ACK;
    tcp_hdr->rwnd = htons(TCP_DEFAULT_WINDOW);

    memcpy(pkt + hdr_len, payload, pl_len);

    // Set checksums
    tcp_hdr->checksum = tcp_checksum(ip_hdr, tcp_hdr);
    ip_hdr->checksum = ip_checksum(ip_hdr);

    if(MODE_RELIABLE_SEND == g_config.mode){
        // setting 1: drop packets
        if(rand() % 100 < g_config.loss){
            return 0;
        }
        // setting 2: shuffle packets
        if(rand() % 100 < g_config.shuffle){
            int id = rand() % SHUFFLE_WINDOW;
            if(g_sender.shuffle_packet[id] == NULL){
                g_sender.shuffle_packet[id] = pkt;
                g_sender.shuffle_pkt_len[id] = pkt_len;
                return 0;
            }
            char *temp_pkt = pkt;
            int temp_len = pkt_len;
            pkt = g_sender.shuffle_packet[id];
            pkt_len = g_sender.shuffle_pkt_len[id];
            g_sender.shuffle_packet[id] = temp_pkt;
            g_sender.shuffle_pkt_len[id] = temp_len;
        }
        uint32_t rtp_seq = ntohl(packet_to_rtp_hdr(pkt + hdr_len)->seq);
        uint32_t rtp_len = ntohs(packet_to_rtp_hdr(pkt + hdr_len)->len);
        printf("Info: Send package of seq=%d, \tlen=%d\n", rtp_seq, rtp_len);
    }

    // Send the packet out
    if (0 != pcap_sendpacket(g_handle, (const u_char *)pkt, pkt_len))
    {
        printf("[%s] Error: Cannot send packets due to %s ...\n", \
            __FUNCTION__, pcap_geterr(g_handle));

        free(pkt);
        return 0;
    }

    free(pkt);

    return pl_len;
}

void send_packet_one_epoch(char *payload, int interval, int len)
{
    if(len == 0){
        len = fill_payload(payload, MAX_PAYLOAD_LEN, g_sender.send_cnt);
    }
    if(len == 0){
        return;
    }
    g_sender.send_cnt++;
    int sent_bytes = send_packet(payload, g_sender.seq, 0, len);
    g_sender.seq += sent_bytes;
    if(interval){
        platform_wait_us(interval);
    }
}

static void sender_send(char *input)
{
    char payload[MAX_PAYLOAD_LEN] = {'\0'};

    int num_pkts = atoi(input + 5);
    int interval = (g_config.interval >= 0) ? g_config.interval: 
                                             INTERVAL_SEND;
    if (MODE_RELIABLE_SEND == g_config.mode){
        while ((int)g_sender.send_cnt < num_pkts){
            while((int)g_receiver.ack_seq + g_config.window < (int)get_next_send_seq());
            send_packet_one_epoch(payload, interval, 0);
        }
    }
    else{
        while ((int)g_sender.send_cnt < num_pkts)
        {
            send_packet_one_epoch(payload, interval, 0);
        }
    }
}

static void sender_test(char *input)
{
    char payload[MAX_PAYLOAD_LEN] = {'\0'};
    int interval = (g_config.interval >= 0) ? g_config.interval:
                                            INTERVAL_BENCHMARK;
#ifndef WIN32
    long test_time = ((long)atoi(input + 5)) * 1000000;  // us
    long diff_time = 0;

    struct timeval tv_start;
    struct timeval tv_end;

    gettimeofday(&tv_start, NULL);

    while (1)
    {
        gettimeofday(&tv_end, NULL);

        diff_time = (tv_end.tv_sec - tv_start.tv_sec) * 1000000 \
                  + tv_end.tv_usec - tv_start.tv_usec;
        
        if (diff_time >= test_time)
        {
            break;
        }

        if (MODE_RELIABLE_SEND == g_config.mode){
            while(g_receiver.ack_seq + g_config.window < get_next_send_seq()){
                // printf("%d %d %d\n", g_receiver.ack_seq, g_config.window, get_next_send_seq());
            }
            send_packet_one_epoch(payload, interval, 0);
        }
        else{
            send_packet_one_epoch(payload, interval, 0);
        }
    }
#else
    long test_time = ((long)atoi(input + 5)) * 1000;  // ms
    clock_t t_start = clock();

    while (clock() - t_start < test_time)
    {
        if (MODE_RELIABLE_SEND == g_config.mode){
            while((int)g_receiver.ack_seq + g_config.window < (int)get_next_send_seq());
            send_packet_one_epoch(payload, interval, 0);
        }
        else{
            send_packet_one_epoch(payload, interval, 0);
        }
    }
#endif  // !WIN32

    printf("Info: Successfully send %d packets!\n", g_sender.send_cnt);
}

static void sender_send_receive(void)
{
    char payload[MAX_PAYLOAD_LEN] = {'\0'};
    int interval = (g_config.interval > 0) ? g_config.interval :
                                             INTERVAL_SEND;

    while ((int)g_sender.send_cnt < g_config.num_pkts)
    {
        send_packet_one_epoch(payload, interval, 0);
    }

    printf("Info: Successfully send %d packets!\n", g_sender.send_cnt);
}

#ifndef WIN32
void *sender_thread(void *param)
#else
unsigned int WINAPI sender_thread(void *param)
#endif  // !WIN32
{
    // Initialize the sender
    g_sender.send_cnt = 0;
    g_sender.seq = 0;

#ifndef WIN32
    struct timeval tv0;

    gettimeofday(&tv0, NULL);
    srand(tv0.tv_usec);
#else
    clock_t t0 = clock();
    srand(t0);
#endif

    // Start Interactive interface according to the mode selected
    if (MODE_RELIABLE_SEND == g_config.mode){
        sender_test("test 3600\n");
    }
    if (MODE_SEND == g_config.mode)
    {
        char input[1000] = {'\0'};
        int c;
        int i = 0;

        // Print usage 
        printf("Info: Input command. For example:\n"
            "\t--- 'send 60': send 60 packets to the opposite\n"
            "\t--- 'test 60': keep sending packets in 60 seconds\n"
            "\t--- 'quit': quit this program\n");

        while (1)
        {
            printf("> ");
            while ((c = getchar()) != '\n' && c != EOF)
            {
                input[i++] = (char)c;
            }
            input[i] = '\0';

            // Parse command
            if (0 == strlen(input))  // single '\n'
            {
                printf("\n");
                continue;
            }

            if (0 == memcmp(input, "send ", 5))
            {
                sender_send(input);
            }
            else if (0 == memcmp(input, "test ", 5))
            {
                sender_test(input);
            }
            else if (0 == memcmp(input, "quit", 4))
            {
                break;
            }
            else
            {
                printf("Error: Invalid command \"%s\"!\n", input);
            }

            g_sender.send_cnt = 0;
            i = 0;
        }
    }
    else if (MODE_SEND_RECEIVE == g_config.mode)  // Just send
    {
        sender_send_receive();
    }
    else
    {
        printf("[%s] Error: Cannot create sender thread under mode %d\n", \
            __FUNCTION__, g_config.mode);
        exit(EXIT_FAILURE);
    }

    printf("Info: Sender finishes its task and exits!\n");

#ifndef WIN32
    return NULL;
#else
    CloseHandle(g_sender.pid);
    return 0;
#endif  // !WIN32
}