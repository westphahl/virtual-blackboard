#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"
#include "mq.h"
#include "logger.h"

int main(int argc, char **argv) {
    /*
    if(argc != 1) {
        fprintf(stderr, "Es wird ein Parameter benötigt");
        exit(1);
    }
    */

    int mq_id;
    struct logmessage buffer;
    FILE* file;

    fprintf(stdout, "opened file");
    /* Open logfile */
    if((file = fopen("log.txt", "a")) == NULL) {
        perror("fopen");
        exit(1);
    }

    /* Open message queue */
    mq_id = get_mq(LOGGER_MQ_KEY);

    /* Wait for log in message queue */
    while(1) {
        //if(msgrcv(mq_id, &buffer, MSGSIZE, MSGTYPE_INFO, 0) < 0) {
        //    perror("msgrcv");
        //    exit(1);
        //}
        buffer = read_mq(mq_id);

        switch(buffer.level) {
        case 1:
            fprintf(file, "[ERROR] %ld %s \n", buffer.time, buffer.message);
            break;
        case 2:
            fprintf(file, "[INFO] %ld %s \n", buffer.time, buffer.message);
            break;
        case 3:
            fprintf(file, "[DEBUG] %ld %s \n", buffer.time, buffer.message);
            break;
        }

        fflush(file);
    }
    fclose(file);
}
