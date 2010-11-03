#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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

static pthread_mutex_t trigger_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t trigger_bcast = PTHREAD_COND_INITIALIZER;
enum broadcast_t {STATUS, BLACKBOARD};
static int broadcast_type = STATUS;

void trigger_broadcast(int type) {
    pthread_mutex_lock(&trigger_mutex);
    broadcast_type = type;
    pthread_cond_signal(&trigger_bcast);
    pthread_mutex_unlock(&trigger_mutex);
}

void trigger_status() {
    trigger_broadcast(STATUS);
}

void trigger_blackboard() {
    trigger_broadcast(BLACKBOARD);
}

void broadcast_status() {
    struct cl_entry *current;
    struct net_status *status;
    uint8_t dcount;
    uint16_t scount;
    uint8_t tcount;
    uint8_t permission;
    int ret;
    
    current = start_iteration();
    dcount = docent_exists();
    scount = get_client_count();
    tcount = tutor_exists();

    while (current != NULL) {
        if (current == get_write_user()) {
            permission = 1;
        } else {
            permission = 0;
        }
        status = build_status(current->cdata->role, current->cdata->cid,
                permission, dcount, tcount, scount);
        ret = send(current->cdata->sfd, status, sizeof(struct net_status), 0);
        if (ret < 0) {
            perror("send");
        }
        free(status);
        current = iteration_next();
    }

    log_debug("broadcasting agent: status sent to all connected clients");
    end_iteration();
}

void broadcast_blackboard(char *blackboard, int bsem_id) {
    struct cl_entry *current;
    struct net_board *board;
    int ret;
    int length;

    // Prepare the board message
    lock_bb_sem(bsem_id);
    length = strlen(blackboard);
    board = build_board(blackboard, length);
    unlock_bb_sem(bsem_id);   
    
    current = start_iteration();

    while (current != NULL) {
        if (current == get_write_user()) {
            current = iteration_next();
            continue;
        }
        ret = send(current->cdata->sfd, board, sizeof(struct net_header) + length, 0);
        if (ret < 0) {
            perror("send");
        }
        current = iteration_next();
    }
    
    free(board);
    log_debug("broadcasting agent: blackboard sent to all connected clients");
    end_iteration();
   
}

void* broadcasting_agent(void *arg) {
    int bsem_id;
    key_t bsem_key = BLACKBOARD_SEM_KEY;

    bsem_id = get_bb_sem(bsem_key);

    int bshm_id;
    key_t bshm_key = BLACKBOARD_SHM_KEY;

    bshm_id = get_blackboard(bshm_key);
    char *blackboard;

    blackboard = blackboard_attach(bshm_id);

    log_info("broadcasting agent: waiting for trigger");
    pthread_mutex_lock(&trigger_mutex);
    while(1) {
        pthread_cond_wait(&trigger_bcast, &trigger_mutex);
        switch (broadcast_type) {
            case STATUS:
                log_debug("broadcasting agent: received status trigger");
                broadcast_status();
                break;
            case BLACKBOARD:
                log_debug("broadcasting agent: received blackboard trigger");
                broadcast_blackboard(blackboard, bsem_id);
                break;
        }
    }
    pthread_mutex_unlock(&trigger_mutex);

    pthread_exit(NULL);
}
