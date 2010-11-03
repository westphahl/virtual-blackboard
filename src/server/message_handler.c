#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../commons.h"
#include "../net_message.h"
#include "message_handler.h"
#include "client_list.h"
#include "utils.h"
#include "client_list.h"
#include "broadcasting.h"
#include "blackboard.h"
#include "shared.h"

/*
 * Handles login of client
 * Returns 0 on success and 1 if the client could not
 * be logged in (e.g. non RFC compliant).
 * In case of some socket error -1 is returned.
 *
 * Dealing with errors is up to the caller.
 */
int login_handler(int sfd) {
    struct net_header *header = NULL;
    ssize_t ret;
    struct client_data *cdata = NULL;
    void *rbuf = NULL; // Receive buffer
    char *cname;

    /*
     * Wait for client login
     */
    header = (struct net_header *) malloc(sizeof(struct net_header));
    ret = recv(sfd, header, sizeof(struct net_header), MSG_WAITALL);

    // Close socket if not a login message
    if (header->type != m_login) {
        log_error("login handler: got wrong message type.");
        return 1;
    }

    // Connection closed by client or error
    if (ret <= 0) {
        log_error("login handler: connection closed by client");
        free(header);
        return -1;
    }
    
    // Convert to host byte order
    ntoh_header(header);

    /* 
     * Length must be at least 2 bytes
     * (role == 1 byte; name >= 1)
     */
    if (header->length < 2) {
        log_error("login handler: login message too short.");
        return 1;
    }

    // Allocate memory for receive buffer
    rbuf = malloc(header->length);
    // Receive login message
    ret = recv(sfd, rbuf, header->length, MSG_WAITALL);

    //Connection closed by client or error
    if (ret == 0) {
        log_error("login handler: connection closed by client");
        free(header);
        free(rbuf);
        return -1;
    } else if (ret < 0) {
        log_error("login handler: connection error");
        free(header);
        return -1;
    }


    // Allocate memory for client login message
    cdata = (struct client_data *) malloc(sizeof(struct client_data));

    cdata->cid = get_next_cid();
    cdata->sfd = sfd;

    // Copy role
    memcpy(&cdata->role, rbuf, sizeof(uint8_t));

    // Read name
    cname = (char *) malloc(header->length);

    // Terminate client name
    cname[header->length - 1] = 0;
    memcpy(cname, (char *) rbuf + sizeof(uint8_t), header->length - sizeof(uint8_t));

    switch (cdata->role) {
        case INDIFFERENT:
            if (docent_exists()) {
                cdata->role = STUDENT;
            } else {
                cdata->role = DOCENT;
            }
            break;
        case STUDENT:
            break;
        case DOCENT:
            if (docent_exists()) {
                cdata->role = STUDENT;
            }
            break;
        default:
            log_error("login handler: invalid role.");
            cdata->role = STUDENT;
    }

    add_client(cdata);
    trigger_status();

    log_info("login handler: client login successful");
    free(header);
    free(rbuf);

    return 0;
}

/*
 * Handles board update sent by client
 */
int board_handler(int sfd, uint16_t length) {

    if (has_write_access(sfd) < 1) {
        log_error("board handler: user without write access");
        return -1;
    }
    struct net_board *board;
    int ret = 1;

    key_t bsem_key = BLACKBOARD_SEM_KEY;
    int bsem_id = get_bb_sem(bsem_key);

    char *blackboard;
    key_t bshm_key = BLACKBOARD_SHM_KEY;
    int bshm_id = get_blackboard(bshm_key);

    // Attach blackboard
    blackboard = blackboard_attach(bshm_id);


    lock_bb_sem(bsem_id);
    memset(blackboard, 0, BLACKBOARD_BYTESIZE);

    // If not a delete message
    if (length > 0) {
        board = (struct net_board *) malloc(length + sizeof(struct net_header));
        if (board == NULL) {
            unlock_bb_sem(bsem_id);
            log_error("board handler: malloc failed");
            exit(EXIT_FAILURE);
        }
        ret = recv(sfd, board->content, length, MSG_WAITALL);

        // Connection closed by client or error
        if (ret == 0) {
            unlock_bb_sem(bsem_id);
            free(board);
            log_info("board handler: connection closed by client");
            return -1;
        } else if (ret < 0) {
            unlock_bb_sem(bsem_id);
            free(board);
            log_error("board handler: connection error");
            return -1;
        }

        memcpy(blackboard, board->content, length);
        fprintf(stdout, "blackboard: %s\n", blackboard);
        free(board);
    }

    unlock_bb_sem(bsem_id);
    blackboard_detach(blackboard);
    trigger_blackboard();
    return 0;
}
