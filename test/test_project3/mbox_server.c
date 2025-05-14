#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <mailbox.h>

#define STR_MBOX "str-message-mbox"
#define POS_MBOX "pos-message-mbox"

static const char initReq[] = "clientInitReq";
static const int initReqLen = sizeof(initReq);

static int clientInitReq(const char* buf, int length)
{
    if (length != initReqLen) return 0;
    for (int i = 0; i < initReqLen; ++i) {
        if (buf[i] != initReq[i]) return 0;
    }
    return 1;
}


int main(int argc, char *argv[])
{
#ifndef S_CORE
    assert(argc >= 1);
    int print_location = (argc == 1) ? 0 : atoi(argv[1]);

    // open two mailboxs
    int handle_mq = sys_mbox_open(STR_MBOX);
    int handle_posmq = sys_mbox_open(POS_MBOX);    

    char msgBuffer[MAX_MBOX_LENGTH];
    MsgHeader_t header;
    int64_t correctRecvBytes = 0;
    int64_t errorRecvBytes = 0;
    int64_t blockedCount = 0;
    int clientPos = print_location + 1;


    sys_move_cursor(0, print_location);
    printf("[Server] server started");
    sys_sleep(1);

    for (;;)
    {
        blockedCount += sys_mbox_recv(handle_mq, &header, sizeof(MsgHeader_t));
        blockedCount += sys_mbox_recv(handle_mq, msgBuffer, header.length);

        uint32_t checksum = adler32(msgBuffer, header.length);
        if (checksum == header.checksum) {
            correctRecvBytes += header.length;
        } else {
            errorRecvBytes += header.length;
        }

        sys_move_cursor(0, print_location);
        printf("[Server]: recved msg from %d (blocked: %ld, correctBytes: %ld, errorBytes: %ld)",
              header.sender, blockedCount, correctRecvBytes, errorRecvBytes);

        if (clientInitReq(msgBuffer, header.length)) {
            sys_mbox_send(handle_posmq, &clientPos, sizeof(int));
            ++clientPos;
        }

        sys_sleep(1);
    }
#endif

    return 0;
}
