#include <sys/sem.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>

union semun {
    int val;
    struct semid_ds *buf;
    ushort *array;
};

/*
 * Function: int init_sem(key_t key)
 * Description: Creates a new semaphore for regulating access
 * to the shared memory segment.
 */
int init_sem(key_t key) {
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
 * Function: int get_sem(key_t key)
 * Description: Gets the specified semaphore
 */
int get_sem(key_t key) {
    int semid;

    if ((semid = semget(key, 1, 0600)) < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    return semid;
}

/* 
 * Function: void lock_sem(int semid)
 * Description: Lock the specified semaphore
 */
void lock_sem(int semid) {
    struct sembuf lock_buf;
    lock_buf.sem_num = 0;
    lock_buf.sem_op = -1;
    lock_buf.sem_flg = 0;
    if (semop(semid, &lock_buf, 1) < 0) {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: void unlock_sem(int semid)
 * Description: Unlock the specified semaphore
 */
void unlock_sem(int semid) {
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
 * Function: void wait_sem(int semid)
 * Description: Wait for the specified semaphore to become zero
 */
void wait_sem(int semid) {
    struct sembuf wait_buf;
    wait_buf.sem_num = 0;
    wait_buf.sem_op = 0;
    wait_buf.sem_flg = 0;
    if (semop(semid, &wait_buf, 1) < 0) {
        perror("semop");
        exit(EXIT_FAILURE);
    }
}

/*
 * Function: void delete_sem(int semid)
 * Description: Removes the specified semaphore
 */
void delete_sem(int semid) {
    if (semctl(semid, 0, IPC_RMID) < 0) {
        perror("semctl");
    }
}

