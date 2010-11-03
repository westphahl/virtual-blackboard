/*
 * Module: blackboard
 * Type: Function library
 * Description: Helper functions for working with the blackboard
 * in shared memory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "blackboard.h"
#include "../commons.h"

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

/*
 * Function: int init_bb_semaphore(key_t key)
 * Description: Creates a new semaphore for regulating access
 * to the shared memory segment.
 */
int init_bb_sem(key_t key) {
    int semid;
    union semun arg;

    if ((semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0600)) < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) < 0) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    return semid;
}

/*
 * Function: int get_bb_sem(key_t key)
 * Description: Gets the specified semaphore
 */
int get_bb_sem(key_t key) {
    int semid;

    if ((semid = semget(key, 1, 0600)) < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    return semid;
}

void lock_bb_sem(int semid) {
    struct sembuf lock_buf;
    lock_buf.sem_num = 0;
    lock_buf.sem_op = -1;
    lock_buf.sem_flg = 0;
    if (semop(semid, &lock_buf, 1) < 0) {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

void unlock_bb_sem(int semid) {
    struct sembuf unlock_buf;
    unlock_buf.sem_num = 0;
    unlock_buf.sem_op = 1;
    unlock_buf.sem_flg = 0;
    if (semop(semid, &unlock_buf, 1) < 0) {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: void delete_bb_sem(int semid)
 * Description: Removes the specified semaphore
 */
void delete_bb_sem(int semid) {
    if (semctl(semid, 0, IPC_RMID) < 0) {
        perror("semctl");
    }
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

char* blackboard_attach(int shmid) {
    char *bboard;

    if ((bboard = shmat(shmid, (void *) 0, 0)) < (char *) 0) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    return bboard;
}

void blackboard_detach(char *bboard) {
    if (shmdt((void *) bboard) < 0) {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: int init_blackboard(key_t key)
 * Description: Creates a new shared memory segment for the blackboard
 * and returns the id of the segment.
 */
int init_blackboard(key_t key) {
    int shmid;
    char *board;
    char welcome[] = "Welcome!\0";

    if ((shmid = shmget(key, BLACKBOARD_BYTESIZE, IPC_CREAT | IPC_EXCL | 0600)) < 0) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    board = blackboard_attach(shmid);

    // Initialize shared memory with 0
    memset(board, 0, BLACKBOARD_BYTESIZE);

    /*
     * Copy welcome string
     * The last byte should be 0, so only 1199 Bytes are copied
     */
    strncpy(board, welcome, BLACKBOARD_BYTESIZE - 1);

    fprintf(stdout, "Segment contains: %s\n", board);
    blackboard_detach(board);

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
    }
}
