#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "blackboard.h"
#include "../commons.h"

int init_blackboard(key_t key) {
    int shmid;

    if ((shmid = shmget(key, BLACKBOARD_BYTESIZE, IPC_CREAT | IPC_EXCL | 0600)) < 0) {
        perror("shmget");
        exit(1);
    }
    return shmid;
}

int get_blackboard(key_t key) {
    int shmid;

    if ((shmid = shmget(key, BLACKBOARD_BYTESIZE, 0)) < 0) {
        perror("shmget");
        exit(1);
    }
    return shmid;
}

void delete_blackboard(int shmid) {
    // Shared memory data structure to hold results
    struct shmid_ds shm_ds; 

    if ((shmctl(shmid, IPC_RMID, &shm_ds) < 0)) {
        perror("shmctl");
        exit(1);
    }
}
