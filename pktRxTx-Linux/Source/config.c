#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#ifndef WIN32
#include <unistd.h>
#include <pcap.h>
#else
#include "pcap.h"
#endif  // !WIN32

#include "common.h"

config_t g_config;

#ifndef WIN32
extern char *optarg;
#endif  // !WIN32

static void print_usage(void)
{
    printf("Usage: pktRxTx [Options] ...\n"
        "Options:\n"
        "\t-h: Print usage information.\n"
        "\t-m <mode>:\n"
        "\t\t1. SEND: Send packets to the opposite;\n"
        "\t\t2. SEND_RECEIVE: Send packets to the opposite, "
        "while awaiting packets from the opposite;\n"
        "\t\t3. RECEIVE: Receive packets from the opposite;\n"
        "\t\t4. RECEIVE_SEND: Receive packets from the opposite, "
        "and re-transmitted these packets back to the opposite;\n"
        "\t\t5. SEND_RELIABLE: Send packets to the opposite "
        "with a reliable transport layer protocal;\n"
        "\t\t6. RECEIVE_RELIABLE: RECEIVE packets from the opposite "
        "with a reliable transport layer protocal;\n"
        "\t-n <number>: Total amount of packets to be sent out.\n"
        "\t-t <seconds>: Interval of sending packets continuously.\n"
        "\t-w <number>: Size of sending window in SEND_RELIABLE mode.\n"
        "\t-p <number>: Print result when receiving new <number> packets\n"
        "\t-f <path>: File to send in SEND_RELIABLE mode.\n"
        "\t-l <number>: <number>/100 of packets may be lost in SEND_RELIABLE mode.\n"
        "\t-s <number>: <number>/100 of packets may be shuffled in SEND_RELIABLE mode.\n"
    );
}

static void parse_and_check(int argc, char *argv[])
{
    bool set_mode = false;
    bool set_num_pkts = false;
    bool set_interval = false;
    bool set_window = false;
    bool set_rcv_pkt_interval = false;
    bool set_file_name = false;
    bool set_loss = false;
    bool set_shuffle = false;

    // Parse CMD Arguments
#ifndef WIN32
    int res = -1;

    while (-1 != (res = getopt(argc, argv, "hm:n:t:w:p:f:l:s:")))
    {
        switch (res)
        {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
                break;
            case 'm':
                g_config.mode = atoi(optarg);
                set_mode = true;

                if (MODE_INVALID == g_config.mode \
                    || MAX_MODES <= g_config.mode)
                {
                    printf("[%s] Error: Mode out of range [1, %d)!\n", 
                            __FUNCTION__, MAX_MODES);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'n':
                g_config.num_pkts = atoi(optarg);
                set_num_pkts = true;

               if (g_config.num_pkts <= 0)
                {
                    printf("[%s] Error: Num_pkts must be positive!\n", \
                            __FUNCTION__);
                    exit(EXIT_FAILURE);
                }
                break;
            case 't':
                g_config.interval = atoi(optarg);
                set_interval = true;

                if (g_config.interval < 0)
                {
                    printf("[%s] Error: Interval must be non-negative!\n", \
                            __FUNCTION__);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'w':
                g_config.window = atoi(optarg);
                set_window = true;

                if (g_config.window <= 0)
                {
                    printf("[%s] Error: Window must be positive!\n", \
                            __FUNCTION__);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                g_config.rcv_pkt_interval = atoi(optarg);
                set_rcv_pkt_interval = true;

                if (g_config.rcv_pkt_interval <= 0)
                {
                    printf("[%s] Error: Pkt_interval must be positive!\n", \
                            __FUNCTION__);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'f':
                strcpy(g_config.filename, optarg);
                FILE * fp = fopen(g_config.filename, "r");
                if(!fp){
                    printf("[%s] Error: Open file failed!\n", \
                            __FUNCTION__);
                    exit(EXIT_FAILURE);
                }
                fseek(fp, 0, SEEK_END);
                g_config.file_size = ftell(fp);
                printf("Info: filesize %d\n", g_config.file_size);
                fclose(fp);
                set_file_name = true;
                break;
            case 'l':
                g_config.loss = atoi(optarg);
                set_loss = true;

                if (g_config.loss < 0 || g_config.loss >= 100)
                {
                    printf("[%s] Error: Loss rate must be 0-99!\n", \
                            __FUNCTION__);
                    exit(EXIT_FAILURE);
                }
                break;
            case 's':
                g_config.shuffle = atoi(optarg);
                set_shuffle = true;

                if (g_config.shuffle < 0 || g_config.shuffle >= 100)
                {
                    printf("[%s] Error: Shuffle rate must be 0-99!\n", \
                            __FUNCTION__);
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                printf("[%s] Error: Detect Invalid Argument -%c! "
                        "See help below for more information!\n", \
                        __FUNCTION__, res);
                print_usage();
                exit(EXIT_FAILURE);
                break;
        }
    }
#else
    for (int i = 1; i < argc; ++i)
    {
        char *p = argv[i];
        if (*p++ != '-')
        {
            printf("[%s] Error: cmd argument should start with '-'!\n", \
                    __FUNCTION__);
            exit(EXIT_FAILURE);
        }

        while (*p) 
        {
            switch (*p++) 
            {
            case 'h':  // help
                print_usage();
                exit(EXIT_SUCCESS);
                break;
            case 'm':  // mode
                if (argv[++i]) 
                {
                    g_config.mode = atoi((const char *)argv[i]);
                    set_mode = true;
                    if (MODE_INVALID == g_config.mode \
                        || MAX_MODES <= g_config.mode)
                    {
                        printf("[%s] Error: Mode out of range [1, %d)!\n",
                            __FUNCTION__, MAX_MODES);
                        exit(EXIT_FAILURE);
                    }
                    goto next;
                }
                break;
            case 'n':  // num_pkts
                if (argv[++i]) 
                {
                    g_config.num_pkts = atoi((const char *)argv[i]);
                    set_num_pkts = true;
                    if (g_config.num_pkts <= 0)
                    {
                        printf("[%s] Error: Num_pkts must be positive!\n", \
                            __FUNCTION__);
                        exit(EXIT_FAILURE);
                    }
                    goto next;
                }
                break;
            case 't':  // interval
                if (argv[++i]) 
                {
                    g_config.interval = atoi((const char *)argv[i]);
                    set_interval = true;
                    if (g_config.interval < 0)
                    {
                        printf("[%s] Error: Interval must be non-negative!\n", \
                            __FUNCTION__);
                        exit(EXIT_FAILURE);
                    }
                    goto next;
                }
                break;
            case 'w':  // window
                if (argv[++i]) 
                {
                    g_config.window = atoi((const char *)argv[i]);
                    set_window = true;
                    if (g_config.window <= 0)
                    {
                        printf("[%s] Error: Window must be positive!\n", \
                            __FUNCTION__);
                        exit(EXIT_FAILURE);
                    }
                    goto next;
                }
                break;
            case 'p':  // rcv_pkt_interval
                if (argv[++i]) 
                {
                    g_config.rcv_pkt_interval = atoi((const char *)argv[i]);
                    set_rcv_pkt_interval = true;
                    if (g_config.rcv_pkt_interval <= 0)
                    {
                        printf("[%s] Error: Pkt_interval must be positive!\n", \
                            __FUNCTION__);
                        exit(EXIT_FAILURE);
                    }
                    goto next;
                }
                break;
            case 'f':
                if (argv[++i]){
                    strcpy(g_config.filename, argv[i]);
                    FILE * fp = fopen(g_config.filename, "r");
                    if(!fp){
                        printf("[%s] Error: Open file failed!\n", \
                                __FUNCTION__);
                        exit(EXIT_FAILURE);
                    }
                    fseek(fp, 0, SEEK_END);
                    g_config.file_size = ftell(fp);
                    printf("Info: filesize %d\n", g_config.file_size);
                    fclose(fp);
                    set_file_name = true;
                    goto next;
                }
                break;
            case 'l':  // window
                if (argv[++i]) 
                {
                    g_config.loss = atoi((const char *)argv[i]);
                    set_loss = true;
                    if (g_config.loss < 0 || g_config.loss >= 100)
                    {
                        printf("[%s] Error: Loss rate must be 0-99!\n", \
                            __FUNCTION__);
                        exit(EXIT_FAILURE);
                    }
                    goto next;
                }
                break;
             case 's':  // window
                if (argv[++i]) 
                {
                    g_config.shuffle = atoi((const char *)argv[i]);
                    set_shuffle = true;
                    if (g_config.shuffle < 0 || g_config.shuffle >= 100)
                    {
                        printf("[%s] Error: Shuffle rate must be 0-99!\n", \
                            __FUNCTION__);
                        exit(EXIT_FAILURE);
                    }
                    goto next;
                }
                break;
            default:
                printf("[%s] Error: Detect Invalid Argument! " \
                    "See help below for more information!\n", \
                    __FUNCTION__);
                print_usage();
                exit(EXIT_FAILURE);
                break;
            }
        }
    next:
        continue;
    }
#endif  // !WIN32

    // Check whether necessary arguments are set, if not, use default value
    if (!set_mode)
    {
        printf("[%s] Error: Mode should be set!\n", __FUNCTION__);
        exit(EXIT_FAILURE);
    }

    if (!set_num_pkts && MODE_SEND_RECEIVE == g_config.mode)
    {
        printf("[%s] Warning: num_pkts is not set!\n", __FUNCTION__);
        g_config.num_pkts = 0;
    }

    if (!set_interval)
    {
        g_config.interval = -1;  // Use default interval (defined as macro)
    }

    if (!set_window)
    {
        g_config.window = RTP_DEFAULT_WINDOW;
    }

    if (!set_file_name)
    {
        g_config.filename[0] = '\0';
        g_config.file_size = 0;
    }

    if (!set_loss){
        g_config.loss = 0;
    }

    if (!set_shuffle){
        g_config.shuffle = 0;
    }

    if (!set_rcv_pkt_interval)
    {
        g_config.rcv_pkt_interval = DEFAULT_PKT_INTERVAL;
    }
}

static pcap_if_t *print_dev_list(int *num_dev)
{
    pcap_if_t *alldevs = NULL;
    char errbuf[PCAP_ERRBUF_SIZE] = {'\0'};

    if (-1 == pcap_findalldevs(&alldevs, errbuf))
    {
        printf("[%s] Error: pcap_findalldevs failed due to \"%s\" ...\n", \
            __FUNCTION__, errbuf);
        exit(EXIT_FAILURE);
    }

    // Print the Device List
    int i = 0;
    for (pcap_if_t *d = alldevs; NULL != d; d = d->next)
    {
        if (NULL != d->description)
        {
            printf("%d. %s\n", ++i, d->description);
        }
        else
        {
            printf("%d. %s (No Description Avaliable)\n", ++i, d->name);
        }
    }

    if (0 == i)
    {
        printf("[%s] Error: Cannot find any interfaces!\n", __FUNCTION__);
        pcap_freealldevs(alldevs);
        exit(EXIT_FAILURE);
    }

    *num_dev = i;

    return alldevs;
}

static void select_device(void)
{
    int num_dev;
    pcap_if_t *alldevs = print_dev_list(&num_dev);

    printf("Enter the interface number (1-%d): ", num_dev);

    int c;
    int target_idx = 0;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        if (isdigit(c))
        {
            target_idx = target_idx * 10 + c - '0';
        }
        else
        {
            printf("[%s] Error: Invalid input!\n", __FUNCTION__);
            pcap_freealldevs(alldevs);
            exit(EXIT_FAILURE);
        }
    }

    if (target_idx < 1 || target_idx > num_dev)
    {
        printf("[%s] Error: Interface number out of range!\n", \
            __FUNCTION__);
        pcap_freealldevs(alldevs);
        exit(EXIT_FAILURE);
    }

    // Jump to the Selected Adapter
    pcap_if_t *d = alldevs;
    for (int i = 0; i < target_idx - 1; ++i)
    {
        d = d->next;
    }

    strcpy(g_config.dev, d->name);
}

static void set_unalterable_configs(void)
{
    // Set fixed src/dst MAC Address
    uint8_t fixed_smac[ETH_ALEN] = { 0x80, 0xfa, 0x5b, 0x33, 0x56, 0xef };
    uint8_t fixed_dmac[ETH_ALEN] = { 0x00, 0x0a, 0x35, 0x00, 0x1e, 0x53 };  // PYNQ
    // uint8_t fixed_dmac[ETH_ALEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }; // Broadcast

    memcpy(g_config.src_mac, fixed_smac, ETH_ALEN * sizeof(uint8_t));
    memcpy(g_config.dst_mac, fixed_dmac, ETH_ALEN * sizeof(uint8_t));

    // Set fixed src/dst IP Address
    g_config.saddr =  10 << 24 |   0 << 16 |   0 << 8 |  67;  // 10.0.0.67
    g_config.daddr = 255 << 24 | 255 << 16 | 255 << 8 | 255;  // 255.255.255.255

    // Set fixed src/dst Port
    g_config.sport  = DEFAULT_SPORT;
    g_config.dport1 = DEFAULT_DPORT1;
    g_config.dport2 = DEFAULT_DPORT2;
}

void load_config(int argc, char *argv[])
{
    memset(&g_config, 0, sizeof(g_config));

    // Start Loading
    parse_and_check(argc, argv);

    // Set other configurations ...
    select_device();

    set_unalterable_configs();
}
