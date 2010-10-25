/*
 * Module: mq
 * Type: Function library
 * Description: Helper functions for creating and getting message queue
 */
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

#include "mq.h"

int create_mq(key_t key) {
    int mq_id;

    if ((mq_id = msgget(key, IPC_CREAT | IPC_EXCL| 0600)) < 0) {
        perror("msgget");
        exit(1);
    }
    return mq_id;
}

int get_mq(key_t key) {
    int mq_id;

    if ((mq_id = msgget(key, 0600)) < 0) {
        perror("msgget");
        exit(1);
    }
    return mq_id;
}

void delete_mq(int mq_id) {
    if (msgctl(mq_id, IPC_RMID, NULL) < 0) {
        perror("msgctl");
        exit(1);
    }
}
