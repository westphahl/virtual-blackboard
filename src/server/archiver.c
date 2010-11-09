#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "shared.h"
#include "blackboard.h"
#include "semaphore.h"

struct thread_data {
    FILE* file;
    char* blackboard;
    int interval;
    int bsem;
};

void archive_blackboard(char *blackboard, FILE* file) {
    /* Get current time */
    time_t timer = time(NULL);
    /* Convert to local time */
    struct tm *now = localtime(&timer);

    /* Write blackboard to file */
    fprintf(file, "### %s", asctime(now));
    fprintf(file, ">>> START\n");
    fprintf(file, "%s\n", blackboard);
    fprintf(file, "<<< END\n");
    /* Flush write buffer */
    fflush(file);
}

void* debug_thread(void *data) {
    struct thread_data *tdata = (struct thread_data *) data;
    
    while(1) {
        lock_sem(tdata->bsem);
        archive_blackboard(tdata->blackboard, tdata->file);
        unlock_sem(tdata->bsem);

        /* Sleep for a while */
        sleep(tdata->interval);
    }
}

/*
 * The Archiver writes the blackboard to a file every time the blackboard
 * is cleard. In debug mode the blackboard is saved periodically
 *
 * The archiver takes the period interval as a optional argument.
 * This forces the archiver into debug mode.
 */
int main(int argc, char **argv) {
    FILE *file;
    int debug = 0;
    /* Id of the thread in debug mode */
    pthread_t debug_tid;
    /* Get the semaphore id for the trigger */
    key_t asem_key = ftok(FTOK_PATH, ASEM_ID);
    int asem_id = get_sem(asem_key);
    /* Get the semaphore id for blackboard access */
    key_t bsem_key = ftok(FTOK_PATH, BSEM_ID);
    int bsem_id = get_sem(bsem_key);
    /* Get id of the shared memory segment */
    key_t bshm_key = ftok(FTOK_PATH, BSHM_ID);
    int bshm_id = get_blackboard(bshm_key);
    /* Attach to the shared memory segment */
    char *blackboard = blackboard_attach(bshm_id);

	/* 
     * Set debug mode
     * (49 is the ASCII representation of 1)
     */
    if ((argc > 1) && (argv[1][0] == 49)) {
        debug = 1;
    }

    file = fopen("blackboard.archiv", "w");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if (debug == 1) {
        struct thread_data tdata;
        /* Prepare thread data */
        tdata.file = file;
        tdata.blackboard = blackboard;
        tdata.interval = 10;
        tdata.bsem = bsem_id;

        /* Create debug thread */
        pthread_create(&debug_tid, NULL, debug_thread, (void *) &tdata);
    }

    fprintf(stdout, "Archiver: process started and waiting for trigger\n");
    fflush(stdout);

    while(1) {
        /* Wait for trigger */
        wait_sem(asem_id);

        /* Lock blackboard */
        lock_sem(bsem_id);

        /* Save blackboard to file */
        archive_blackboard(blackboard, file);

        /* Unlock blackboard */
        unlock_sem(bsem_id);
        
        /* Notify board handler */
        unlock_sem(asem_id);
        /* Reset trigger */
        unlock_sem(asem_id);
    }

    /* Detach from the shared memory segment */
    blackboard_detach(blackboard);
    exit(EXIT_SUCCESS);
}
