#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "../commons.h"
#include "../net_message.h"
#include "message_handler.h"
#include "client_list.h"
#include "utils.h"
#include "client_list.h"
#include "broadcasting.h"
#include "blackboard.h"
#include "semaphore.h"
#include "shared.h"
#include "message_builder.h"

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
    if (header == NULL) {
        exit(EXIT_FAILURE);
    }
    ret = recv(sfd, header, sizeof(struct net_header), MSG_WAITALL);

    // Close socket if not a login message
    if (header->type != m_login) {
        free(header);
        log_error("login handler: got wrong message type.");
        return 1;
    }

    // Connection closed by client or error
    if (ret <= 0) {
        free(header);
        log_error("login handler: connection closed by client");
        return -1;
    }
    
    // Convert to host byte order
    ntoh_header(header);

    /* 
     * Length must be at least 2 bytes
     * (role == 1 byte; name >= 1)
     */
    if (header->length < 2) {
        free(header);
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
    cdata = (struct client_data *) malloc(sizeof(struct client_data) +
            header->length);

    memset(cdata, 0, sizeof(struct client_data) + header->length);

    cdata->cid = get_next_cid();
    cdata->sfd = sfd;

    // Copy role
    memcpy(&cdata->role, rbuf, header->length);

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
 * Returns 0 if the board message was processed normaly,
 * otherwise -1.
 * If the length argument is 0, the board is cleared.
 *
 * Dealing with errors is up to the caller.
 */
int board_handler(int sfd, uint16_t length, int mtype) {
    struct net_board *board;
    int ret = 1;

    /*
     * Return if the client has not write access.
     * This may happen from time to time due to some lag.
     */
    if (has_write_access(sfd) < 1) {
        log_error("board handler: user without write access");
        return 0;
    }

    if (mtype == m_clear) {
        // TODO
        // Trigger archivierer
    }

    /* Get semaphore for blackboard access */
    key_t bsem_key = BLACKBOARD_SEM_KEY;
    int bsem_id = get_sem(bsem_key);

    /* Get id of blackboard in shared memory */
    char *blackboard;
    key_t bshm_key = BLACKBOARD_SHM_KEY;
    int bshm_id = get_blackboard(bshm_key);

    /* Attach blackboard in shared memory */
    blackboard = blackboard_attach(bshm_id);

    /* Lock the blackboard semaphore */
    lock_sem(bsem_id);
    /*
     * Reset the blackboard
     * This is required since the shared memory is
     * messed up otherwise somehow :)
     */
    memset(blackboard, 0, BLACKBOARD_BYTESIZE);

    /* This is only required if the length is > 0 */
    if (length > 0) {
        board = (struct net_board *) malloc(length + sizeof(struct net_header));
        if (board == NULL) {
            unlock_sem(bsem_id);
            log_error("board handler: malloc failed");
            exit(EXIT_FAILURE);
        }

        /* Read the board content */
        ret = recv(sfd, board->content, length, MSG_WAITALL);

        /* Check if there was an error */
        if (ret < 0) {
            unlock_sem(bsem_id);
            free(board);
            log_info("board handler: connection error or closed by client");
            return -1;
        }

        /* Copy the board content into the shared memory segment */
        memcpy(blackboard, board->content, length);
        // TODO Debug output
        fprintf(stdout, "blackboard: %s\n", blackboard);
        free(board);
    }

    /* Unlock the blackboard semaphore */
    unlock_sem(bsem_id);
    /* Detach the shared memory segment */
    blackboard_detach(blackboard);

    /* Trigger broadcast of blackboard of clear */
    if (mtype == m_board) {
        trigger_blackboard();
    } else if (mtype == m_clear) {
        trigger_clear();
    }
    return 0;
}

/*
 * Handles a request to change privileges
 */
int request_handler(int sfd) {
    struct net_request *request;
    struct net_query *query;
    struct cl_entry *user;
    struct cl_entry *docent;
    int ret;
    int length;

    request = (struct net_request *) malloc(sizeof(struct net_request));
    if (request == NULL) {
        exit(EXIT_FAILURE);
    }
    ret = recv(sfd, &request->write, sizeof(uint8_t), MSG_WAITALL);

    // Connection closed by client or error
    if (ret == 0) {
        log_error("login handler: connection closed by client");
        free(request);
        return -1;
    } else if (ret < 0) {
        log_error("login handler: connection error");
        free(request);
        return -1;
    }

    lock_clientlist();

    /* Check if user allready has write access */
    if (request->write == has_write_access(sfd)) {
        unlock_clientlist();
        free(request);
        return 0;
    }

    free(request);
    docent = get_docent();

    if (is_docent(sfd)) {
        /*
         * Docent requests write privilege
         */
        user = get_write_user();
        set_write_user(docent);

        // TODO
        // Notify old client with write access

        trigger_status();
    } else {
        /* Get requesting user */
        user = get_user(sfd);
        if (user == NULL) {
            unlock_clientlist();
            return -1;
        }
        length = strlen(user->cdata->name);
        query = build_query(user->cdata->cid, user->cdata->name, length);
        ret = send(docent->cdata->sfd, query, sizeof(struct net_query) + length, 0);
        if (ret < 0) {
            perror("send");
            unlock_clientlist();
            return -1;
        }
        free(query);
    }
    unlock_clientlist();
    return 0;
}
