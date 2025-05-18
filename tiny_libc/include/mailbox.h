#ifndef __INCLUDE_MAILBOX_H__
#define __INCLUDE_MAILBOX_H__

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_MBOX_LENGTH (64)

typedef struct MsgHeader
{
    int length;
    int32_t checksum;
    pid_t sender;
} MsgHeader_t;

uint32_t adler32(char *data, size_t len);
void generateRandomString(char* buf, int len);



#endif