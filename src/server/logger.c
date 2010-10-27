#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mq.h"
#include "logger.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Es wird ein Parameter benötigt");
        exit(1);
    }
    key_t mq_key = atol(argv[1]);
    int mq_id;
    LOGMESSAGE buffer;
    FILE *file;

    /* Open logfile */
    if((file = fopen("log.txt", "a")) == NULL) {
        perror("fopen");
        exit(1);
    }

    /* Open message queue */
    mq_id = get_mq(mq_key);

    /* Wait for log in message queue */
    while(1) {
        if(msgrcv(mq_id, &buffer, MSGSIZE, MSGTYPE, 0) < 0) {
            perror("msgrcv");
            exit(1);
        }
        fprintf(stdout, "%ld %s \n", buffer.time, buffer.message);
        fflush(stdout);
        fprintf(file, "%ld %s \n", buffer.time, buffer.message);
        fflush(file);
    }
}
