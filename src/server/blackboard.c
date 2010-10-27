/*
 * Module: blackboard
 * Type: Function library
 * Description: Helper functions for working with the blackboard
 * in shared memory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "blackboard.h"
#include "../commons.h"

/*
 * Function: int init_blackboard(key_t key)
 * Description: Creates a new shared memory segment for the blackboard
 * and returns the id of the segment.
 */
int init_blackboard(key_t key) {
    int shmid;

    if ((shmid = shmget(key, BLACKBOARD_BYTESIZE, IPC_CREAT | IPC_EXCL | 0600)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

/*
 * Function: int get_blackboard(key_t key)
 * Description: Gets the id of an existing shared memory segment
 * that holds the blackboard, specified by the given key.
 */
int get_blackboard(key_t key) {
    int shmid;

    if ((shmid = shmget(key, BLACKBOARD_BYTESIZE, 0)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

/*
 * Function: int init_blackboard(key_t key)
 * Description: Deletes the shared memory segment specified by
 * the shared memory id.
 */
void delete_blackboard(int shmid) {
    // Shared memory data structure to hold results
    struct shmid_ds shm_ds; 

    if ((shmctl(shmid, IPC_RMID, &shm_ds) < 0)) {
        perror("shmctl");
        exit(EXIT_FAILURE);
    }
}
