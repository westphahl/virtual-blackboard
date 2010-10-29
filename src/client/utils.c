#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

int create_semaphore(int key) {
    int sem_id;

    if((sem_id = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666)) < 0) {
        perror("semget");
        exit(EXIT_FAILURE);
    }
    printf("Semaphore erstellt\n");

    return sem_id;
}

void delete_semaphore(int id) {
    if(id >= 0) {
        semctl(id, 1, IPC_RMID, 0);
        printf("Semaphore gelöscht\n");
    } else {
        perror("semctl");
    }
}

void semaphore_up(int id, struct sembuf buffer) {
    buffer.sem_op += 1;

    if(semop(id, &buffer, 1) == -1) {
        perror("semop");
        exit(EXIT_FAILURE);
    }

    printf("MUTEX UP\n");
    fflush(stdout);
}

void semaphore_down(int id, struct sembuf buffer) {
    buffer.sem_op -= 1;

    if(semop(id, &buffer, 1) == -1) {
        perror("semop");
        exit(EXIT_FAILURE);
    }

    printf("MUTEX DOWN\n");
    fflush(stdout);
}
