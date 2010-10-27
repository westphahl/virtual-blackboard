#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mq.h"
#include "logger.h"

struct message {
    long mtype;
    time_t mtime;
    char mtext[80];
};
typedef struct message MESSAGE;

int main(int argc, char **argv) {
    if(argc != 2) {
        fprintf(stderr, "Es wird ein Parameter benötigt");
        exit(1);
    }
    key_t mq_key = atol(argv[1]);
    int mq_id;
    MESSAGE buffer;
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
        if(msgrcv(mq_id, &buffer, sizeof(buffer), 12345, 0) < 0) {
            perror("msgrsv");
            exit(1);
        }
        
        fprintf(file, "%ld %s \n", buffer.mtime, buffer.mtext);
        fflush(file);
    }
}
