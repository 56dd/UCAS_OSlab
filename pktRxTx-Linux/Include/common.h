#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#ifndef WIN32
#include <pcap.h>
#else
#include "pcap.h"
#endif  // !WIN32

#include "platform.h"
#include "protocol.h"
#include "rcv_ofo_buf.h"
#include "send_buf.h"

#include "config.h"
#include "sender.h"
#include "receiver.h"
#include "timer.h"

extern pcap_t *g_handle;

void print_results(void);
void sig_handler(int signo);

#endif  // !__COMMON_H__