#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

#include "message_builder.h"
#include "../net_message.h"

/*
 * Build a status message
 */
struct net_status* build_status(uint8_t role, uint16_t cid,
        uint8_t perm, uint8_t dcount, uint8_t tcount, uint16_t scount) {
    struct net_status *status = NULL;

    status = (struct net_status *) malloc(sizeof(struct net_status));
    if (status == NULL) {
        fprintf(stderr, "message builder: malloc failed");
        exit(EXIT_FAILURE);
    }

    status->header.type = m_status;
    status->header.length = htons((uint16_t) (sizeof(struct net_status)
                - sizeof(struct net_header)));
    status->role = role;
    status->cid = htons(cid);
    status->write = perm;
    status->dcount = dcount;
    status->scount = htons(scount);
    status->tcount = tcount;

    return status;
}

/*
 * Build a board message
 */
struct net_board* build_board(char *content, int length) {
    struct net_board *board = NULL;

    board = (struct net_board *) malloc(sizeof(struct net_status) + length);
    if (board == NULL) {
        fprintf(stderr, "message builder: malloc failed");
        exit(EXIT_FAILURE);
    }

    board->header.type = m_board;
    board->header.length = htons(length);

    memcpy(board->content, content, length);

    return board;
}

/*
 * Build a query message
 * The length parameter specifies the length of name.
 */
struct net_query* build_query(uint16_t cid, char *name, int length) {
    struct net_query *query = NULL;

    query = (struct net_query *) malloc(sizeof(struct net_query) + length);
    if (query == NULL) {
        fprintf(stderr, "message builder: malloc failed");
        exit(EXIT_FAILURE);
    }

    query->header.type = m_query;
    query->header.length = htons(sizeof(uint16_t) + length);

    query->cid = htons(cid);

    memcpy(query->name, name, length);

    return query;
}
