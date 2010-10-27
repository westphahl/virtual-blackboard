#define _XOPEN_SOURCE

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mq.h"
#include "utils.h"

void log_message(char* message) {
    int mq_id;
    LOGMESSAGE buffer;

    mq_id = get_mq(2404);

    buffer.type = MSGTYPE;
    time(&buffer.time);
    strcpy(buffer.message, message);

    if(msgsnd(mq_id, &buffer, MSGSIZE, IPC_NOWAIT) < 0) {
        perror("msgsnd");
        exit(1);
    } else {
        printf("Message \"%s\" sent\n", buffer.message);
    }

    exit(1);
}
