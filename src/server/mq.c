/*
 * Module: mq
 * Type: Function library
 * Description: Helper functions for working with message queue.
 */
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

#include "mq.h"

/*
 * Function: int create_mq(key_t key)
 * Description: Creates a new message queue with the given key 
 * and returns the id of the message queue.
 */
int create_mq(key_t key) {
    int mq_id;

    if ((mq_id = msgget(key, IPC_CREAT | IPC_EXCL| 0666)) < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    return mq_id;
}

/*
 * Function: int get_mq(key_t key)
 * Description: Gets the id of an existing message queue specified
 * by the given key.
 */
int get_mq(key_t key) {
    int mq_id;

    if ((mq_id = msgget(key, 0666)) < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    return mq_id;
}

/*
 * Function: void delete_mq(int mq_id)
 * Description: Delets the message queue specified by the
 * message queue id.
 */
void delete_mq(int mq_id) {
    if (msgctl(mq_id, IPC_RMID, NULL) < 0) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
}
