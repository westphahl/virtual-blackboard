#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "listener_thread.h"
#include "shared.h"
#include "utils.h"

void* listener_handler(void* data) {
    printf("Listener-Thread gestartet \n");
    fflush(stdout);

    /*
    board_sem.sem_op = -1;

    if(-1 == semop(board_sem_id, &board_sem, 1)) {
        perror("semop");
        exit(EXIT_FAILURE);
    }
    */

    semaphore_down(board_sem_id, board_sem);

    getchar();

    /*
    board_sem.sem_op = 1;

    if(-1 == semop(board_sem_id, &board_sem, 1)) {
        perror("semop");
        exit(EXIT_FAILURE);
    }
    */

    semaphore_up(board_sem_id, board_sem);

    pthread_exit(0);
    return NULL;
}
