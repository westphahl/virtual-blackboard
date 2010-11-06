#include <sys/ipc.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "shared.h"
#include "blackboard.h"
#include "semaphore.h"

/*
 * The Archiver writes the blackboard to a file every time the blackboard
 * is cleard. In debug mode the blackboard is saved periodically
 *
 * The archiver takes the period interval as a optional argument.
 * This forces the archiver into debug mode.
 */
int main(int argc, char **argv) {
    /* Get the semaphore id for the trigger */
    int asem_id = get_sem(ARCHIVER_SEM_KEY);
    /* Get the semaphore id for blackboard access */
    int bsem_id = get_sem(BLACKBOARD_SEM_KEY);
    /* Get id of the shared memory segment */
    int bshm_id = get_blackboard(BLACKBOARD_SHM_KEY);
    /* AtTach to the shared memory segment */
    char *blackboard = blackboard_attach(bshm_id);

    // TODO
    // Create a thread if in debug mode

    fprintf(stdout, "Archiver startup complete.\n");
    fflush(stdout);

    while(1) {
        /* Wait for trigger */

        fprintf(stdout, "ARCHIVER: WAITING FOR TRIGGER ...\n");
        fflush(stdout);

        wait_sem(asem_id);
        fprintf(stdout, "ARCHIVER: Got trigger.\n");
        fflush(stdout);

        /* Lock blackboard */
        lock_sem(bsem_id);

        // TODO Call archiver function;
        fprintf(stdout, "ARCHIVER: \"%s\"\n", blackboard);
        fflush(stdout);

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
