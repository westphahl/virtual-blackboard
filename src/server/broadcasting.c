#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <string.h>

#include "message_handler.h"
#include "utils.h"
#include "broadcasting.h"
#include "client_list.h"
#include "../net_message.h"
#include "blackboard.h"
#include "message_builder.h"
#include "shared.h"
#include "semaphore.h"

/* Mutex that protects the condition variable and broadcast type */
static pthread_mutex_t trigger_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t trigger_bcast = PTHREAD_COND_INITIALIZER;
enum broadcast_t {STATUS, BLACKBOARD, CLEAR};
static int broadcast_type = STATUS;

/*
 * Function that triggers the broadcasting agent
 * via a mutex.
 * Not intended for direct usage; use wrapper functions instead.
 */
void trigger_broadcast(int type) {
    pthread_mutex_lock(&trigger_mutex);
    broadcast_type = type;
    pthread_cond_signal(&trigger_bcast);
    pthread_mutex_unlock(&trigger_mutex);
}

/*
 * Wrapper function for triggering the broadcasting agent
 * to send a status message.
 */
void trigger_status() {
    trigger_broadcast(STATUS);
}

/*
 * Wrapper function for triggering the broadcasting agent
 * to send a board message to all clients WITHOUT write access..
 */
void trigger_blackboard() {
    trigger_broadcast(BLACKBOARD);
}

/*
 * Wrapper function for triggering the broadcasting agent
 * to send a board message to all clients;
 */
void trigger_clear() {
    trigger_broadcast(CLEAR);
}

/*
 * Send a status update to all connected clients
 */
void broadcast_status() {
    struct cl_entry *current;
    struct net_status *status;
    uint8_t dcount;
    uint16_t scount;
    uint8_t tcount;
    uint8_t permission;
    int ret;
    
    /*
     * Start iteration over complete client list
     * This locks the mutex for the client list
     */
    current = start_iteration();

    /* Get number of users */
    dcount = docent_exists();
    tcount = tutor_exists();
    scount = get_client_count() - dcount - tcount;

    while (current != NULL) {
        if (current == get_write_user()) {
            permission = 1;
        } else {
            permission = 0;
        }

        /* Build the status message */
        status = build_status(current->cdata->role, current->cdata->cid,
                permission, dcount, tcount, scount);
        ret = send(current->cdata->sfd, status, sizeof(struct net_status), 0);
        if (ret < 0) {
            perror("send");
        }
        free(status);
        /* Select next client in list */
        current = iteration_next();
    }

    log_debug("broadcasting agent: status sent to all connected clients");
    /*
     * End of iteration
     * This unlocks the mutex of the client list
     */
    end_iteration();
}

/*
 * Send a board update message to all connected clients
 *
 * Excludes the user with write acces if the parameter
 * excl_w is set to 1. If the blackbpard should be broadcasted to all
 * connected clients, excl_w shoul be 0.
 */
void broadcast_blackboard(char *blackboard, int bsem_id, int excl_w) {
    struct cl_entry *current;
    struct net_board *board;
    int ret;
    int length;

    /*
     * Prepare the blackboard message
     */
    lock_sem(bsem_id);
    length = strlen(blackboard);
    board = build_board(blackboard, length);
    unlock_sem(bsem_id);
    
    /*
     * Iterate over the complete client list
     * This locks the mutex of the client list
     */
    current = start_iteration();

    while (current != NULL) {
        /* Don't broadcast to the user with write access */
        if ((current == get_write_user()) && excl_w) {
            current = iteration_next();
            continue;
        }
        ret = send(current->cdata->sfd, board, sizeof(struct net_header) + length, 0);
        if (ret < 0) {
            perror("send");
        }
        /* Select next client in list */
        current = iteration_next();
    }
    
    /* Unlock mutex of client list */
    end_iteration();
    free(board);
    log_debug("broadcasting agent: blackboard sent to all connected clients");
}

void* broadcasting_agent(void *arg) {
    int bsem_id;
    key_t bsem_key = ftok(FTOK_PATH, BSEM_ID);
    /* Get the id of the blackboard Semaphore */
    bsem_id = get_sem(bsem_key);

    int bshm_id;
    key_t bshm_key = ftok(FTOK_PATH, BSHM_ID);
    /* Get the id of the blackboard shared memory segment */
    bshm_id = get_blackboard(bshm_key);
    char *blackboard;

    /* Attach to the shared memory */
    blackboard = blackboard_attach(bshm_id);

    log_info("broadcasting agent: waiting for trigger");
    pthread_mutex_lock(&trigger_mutex);
    while(1) {
        /* Wait for the trigger */
        pthread_cond_wait(&trigger_bcast, &trigger_mutex);
        switch (broadcast_type) {
            case STATUS:
                log_debug("broadcasting agent: received status trigger");
                broadcast_status();
                break;
            case BLACKBOARD:
                log_debug("broadcasting agent: received blackboard trigger");
                broadcast_blackboard(blackboard, bsem_id, 1);
                break;
            case CLEAR:
                log_debug("broadcasting agent: received clear trigger");
                broadcast_blackboard(blackboard, bsem_id, 0);
                break;
        }
    }
    pthread_mutex_unlock(&trigger_mutex);
    /* Detach from the shared memory */
    blackboard_detach(blackboard);

    pthread_exit(NULL);
}
