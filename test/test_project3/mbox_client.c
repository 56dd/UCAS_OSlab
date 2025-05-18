#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <mailbox.h>

#define STR_MBOX "str-message-mbox"
#define POS_MBOX "pos-message-mbox"

static const char initReq[] = "clientInitReq";
static const int initReqLen = sizeof(initReq);

static int clientSendMsg(int mq, const char* content, int length)
{
    int i;
    char msgBuffer[MAX_MBOX_LENGTH] = {0};
    MsgHeader_t* header = (MsgHeader_t*)msgBuffer;
    char* _content = msgBuffer + sizeof(MsgHeader_t);
    header->length = length;
    header->checksum = adler32(content, length);
    header->sender = sys_getpid();

    for (i = 0; i < length; ++i) {
        _content[i] = content[i];
    }
    return sys_mbox_send(mq, msgBuffer, length + sizeof(MsgHeader_t));
}

int main()
{
#ifndef S_CORE
    // open two mailboxs
    int handle_mq = sys_mbox_open(STR_MBOX);
    int handle_posmq = sys_mbox_open(POS_MBOX); 

    int len = 0;
    char strBuffer[MAX_MBOX_LENGTH - sizeof(MsgHeader_t)];
    clientSendMsg(handle_mq, initReq, initReqLen);
    int position = 0;
    sys_mbox_recv(handle_posmq, &position, sizeof(int));
    int blocked = 0;
    //int64_t bytes = 0;
    int64_t bytes = initReqLen;

    sys_move_cursor(0, position);
    printf("[Client] server started");
    sys_sleep(1);
    for (;;)
    {
        len = (rand() % ((MAX_MBOX_LENGTH - sizeof(MsgHeader_t))/2)) + 1;
        generateRandomString(strBuffer, len);
        blocked += clientSendMsg(handle_mq, strBuffer, len);
        bytes += len;

        sys_move_cursor(0, position);
        printf("[Client] send bytes: %ld, blocked: %d", bytes, blocked);
        sys_sleep(1);
    }
#endif

    return 0;
}
