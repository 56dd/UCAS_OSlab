#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "ether.h"
#include "ip.h"
#include "tcp.h"
#include "rtp.h"

#define MAX_PAYLOAD_LEN 1000
#define BASE_HDR_LEN	54		// Ethernet + IP + TCP

#endif  // !__PROTOCOL_H__