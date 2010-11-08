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
    struct net_board *board = NULL;
    int ret;
    int length;
    struct client_data *cdata = NULL;
    void *rbuf = NULL; // Receive buffer
    char *blackboard;
    int bshm_id = get_blackboard(BLACKBOARD_SHM_KEY);
    int bsem_id = get_sem(BLACKBOARD_SEM_KEY);

    /*
     * Wait for client login
     */
    header = (struct net_header *) malloc(sizeof(struct net_header));
    if (header == NULL) {
        log_error("login handler: malloc failed");
        exit(EXIT_FAILURE);
    }
    ret = recv(sfd, header, sizeof(struct net_header), MSG_WAITALL);

    /* Close socket if not a login message */
    if (header->type != m_login) {
        free(header);
        log_error("login handler: got wrong message type.");
        return 1;
    }

    /* Connection closed by client or error */
    if (ret <= 0) {
        free(header);
        log_error("login handler: connection closed by client");
        return -1;
    }
    
    /* Convert header to host byte order */
    ntoh_header(header);

    /* 
     * Length must be at least 2 bytes
     * (role == 1 byte; name >= 1)
     */
    if (header->length < 2) {
        free(header);
        log_error("login handler: login message too short.");
        return -1;
    }

    /* Allocate memory for receive buffer */
    rbuf = malloc(header->length);
    if (rbuf == NULL) {
        free(header);
        log_error("login handler: malloc failed");
    }
    /* Receive login message */
    ret = recv(sfd, rbuf, header->length, MSG_WAITALL);

    /* Connection closed by client or error */
    if (ret == 0) {
        log_error("login handler: connection closed by client");
        free(header);
        free(rbuf);
        return -1;
    } else if (ret < 0) {
        log_error("login handler: connection error");
        free(header);
        free(rbuf);
        return -1;
    }

    /* Allocate memory for client data */
    cdata = (struct client_data *) malloc(sizeof(struct client_data) +
            header->length);
    if (cdata == NULL) {
        log_error("login handler: malloc failed");
        exit(EXIT_FAILURE);
    }

    /* Initialize cdata with 0 */
    memset(cdata, 0, sizeof(struct client_data) + header->length);

    /*
     * Get the next available client id
     * get_next_cid() returns 0 if the max number
     * of clients is exceeded
     */
    cdata->cid = get_next_cid();
    if (cdata->cid == 0) {
        struct net_error *error;
        char message[] = "There are too many clients. Sorry!\0";

        log_error("login handler: too many clients");
        error = build_error(e_login, message, strlen(message));

        ret = send(cdata->sfd, error, sizeof(struct net_error) +
                strlen(message), 0);
        if (ret < 0) {
            perror("send");
        }

        free(header);
        free(rbuf);
        free(cdata);
        return -1;
    }
    cdata->sfd = sfd;

    /* Copy role */
    memcpy(&cdata->role, rbuf, header->length);

    /*
     * See if the requested role is available
     *
     * If the role is INDIFFERENT make the client
     * docent if there is no docent, else make student.
     *
     * If the requested role is DOCENT and there is
     * allready a docent make student.
     */
    switch (cdata->role) {
        case INDIFFERENT:
            lock_clientlist();
            if (docent_exists()) {
                cdata->role = STUDENT;
            } else {
                cdata->role = DOCENT;
            }
            unlock_clientlist();
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

    /* Add the client to client list */
    add_client(cdata);

    /* Send client initial blackboard */
    blackboard = blackboard_attach(bshm_id);

    /* Get the current board content */
    lock_sem(bsem_id);
    length = strlen(blackboard);
    board = build_board(blackboard, length);
    unlock_sem(bsem_id);

    /* Detach from the shared memory segment */
    blackboard_detach(blackboard);

    /* Send the initial board */
    ret = send(sfd, board, sizeof(struct net_board) + length, 0);
    if (ret < 0) {
        perror("send");
    }

    /* Trigger a status update */
    trigger_status();

    log_info("login handler: client login successful");
    free(board);
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

    /* Get trigger semaphore for archiver */
    int asem_id = get_sem(ARCHIVER_SEM_KEY);

    /* Get semaphore for blackboard access */
    key_t bsem_key = BLACKBOARD_SEM_KEY;
    int bsem_id = get_sem(bsem_key);

    /* Get id of blackboard in shared memory */
    char *blackboard;
    key_t bshm_key = BLACKBOARD_SHM_KEY;
    int bshm_id = get_blackboard(bshm_key);

    /* Attach blackboard in shared memory */
    blackboard = blackboard_attach(bshm_id);

    /* Check if this is a clear message */
    if (mtype == m_clear) {
        /* Trigger archiver */
        lock_sem(asem_id);
        /* Wait for the archiver to finish */
        lock_sem(asem_id);
    }

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
        fprintf(stdout, "BLACKBOARD CONTENT>>> \n%s\n", blackboard);
        free(board);
    }

    /* Unlock the blackboard semaphore */
    unlock_sem(bsem_id);
    /* Detach the shared memory segment */
    blackboard_detach(blackboard);

    /* Trigger broadcast of blackboard */
    if (mtype == m_board) {
        trigger_blackboard();
    } else if (mtype == m_clear) {
        trigger_clear();
    }
    return 0;
}

/*
 * Handles requests to change privileges
 * Returns -1 on error.
 *
 * Dealing with errors is up to the caller.
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
        log_error("request handler: malloc failed");
        exit(EXIT_FAILURE);
    }
    ret = recv(sfd, &request->write, sizeof(uint8_t), MSG_WAITALL);

    /* Connection closed by client or error */
    if (ret == 0) {
        log_error("request handler: connection closed by client");
        free(request);
        return -1;
    } else if (ret < 0) {
        log_error("request handler: connection error");
        free(request);
        return -1;
    }

    /* Lock the client list */
    lock_clientlist();

    /* Check if user allready has write access */
    if (request->write == has_write_access(sfd)) {
        unlock_clientlist();
        free(request);
        return 0;
    }

    docent = get_docent();

    if (is_docent(sfd)) {
        /*
         * Docent requests write privilege
         */
        user = get_write_user();
        set_write_user(docent);

        struct net_error *error;
        char message[] = "Your write access was revoked by the docent.\0";

        log_info("request handler: write access revoked");
        error = build_error(e_message, message, strlen(message));

        ret = send(user->cdata->sfd, error, sizeof(struct net_error) +
                strlen(message), 0);
        if (ret < 0) {
            perror("send");
        }

        trigger_status();
    } else if (request->write == 0) {
        /*
         * Give back write access
         */
        set_write_user(docent);
        trigger_status();
    } else {
        /*
         * Handle situation wherer no docent exists.
         *
         * A client can gain write access if there is noone with
         * write privileges.
         */
        if (docent_exists() == 0) {
            if (tutor_exists()) {
                unlock_clientlist();
                free(request);
                return 0;
            } else {
                user = get_user_sfd(sfd);
                set_write_user(user);
                unlock_clientlist();
                trigger_status();
                free(request);
                return 0;
            }
        }

        /* Get requesting user */
        user = get_user_sfd(sfd);
        if (user == NULL) {
            free(request);
            unlock_clientlist();
            return -1;
        }

        length = strlen(user->cdata->name);
        query = build_query(user->cdata->cid, user->cdata->name, length);
        ret = send(docent->cdata->sfd, query, sizeof(struct net_query) + length, 0);
        if (ret < 0) {
            free(request);
            perror("send");
            unlock_clientlist();
            return -1;
        }

        log_info("request handler: write access requested");
        free(query);
    }
    free(request);
    unlock_clientlist();
    return 0;
}

/*
 * Handle reply of docent in response to a request by a client
 *
 */
int reply_handler(int sfd) {
    struct net_reply *reply;
    struct cl_entry *user;
    int ret;

    reply = (struct net_reply *) malloc(sizeof(struct net_reply));
    if (reply == NULL) {
        exit(EXIT_FAILURE);
    }
    ret = recv(sfd, &reply->write, sizeof(uint8_t) +
            sizeof(uint16_t),MSG_WAITALL);

    /* Connection closed by client or error */
    if (ret == 0) {
        log_error("reply handler: connection closed by client");
        free(reply);
        return -1;
    } else if (ret < 0) {
        log_error("reply handler: connection error");
        free(reply);
        return -1;
    }

    /* Convert to host byte order */
    reply->cid = ntohs(reply->cid);

    log_debug("reply handler: got reply from docent");
    lock_clientlist();

    if (is_docent(sfd) == 0) {
        free(reply);
        unlock_clientlist();
        log_error("reply handler: non-RFC compliant client kicked");
        return -1;
    }

    user = get_user_cid(reply->cid);

    if (reply->write == 0) {
        /*
         * Notify the client if the request was denied
         */
        struct net_error *error;
        char message[] = "Request denied by docent\0";
        error = build_error(e_message, message, strlen(message));

        ret = send(user->cdata->sfd, error, sizeof(struct net_error) +
                strlen(message), 0);
        if (ret < 0) {
            perror("send");
        }
        free(error);
        free(reply);
        unlock_clientlist();
        log_debug("reply handler: write access denied");
        return 0;
    }

    /* Maybe the user logged out in the meantime */
    if (user == NULL) {
        free(reply);
        unlock_clientlist();
        log_error("reply handler: user not found");
        return 0;
    }

    /* Set the new write user */
    set_write_user(user);
    unlock_clientlist();
    log_debug("reply handler: write access granted");

    free(reply);
    trigger_status();
    
    return 0;
}
