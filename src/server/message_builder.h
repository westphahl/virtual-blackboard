#ifndef MESSAGE_BUILDER_H
#define MESSAGE_BUILDER_H

#include <stdint.h>

struct net_status* build_status(uint8_t role, uint16_t cid,
        uint8_t perm, uint8_t dcount, uint8_t tcount, uint16_t scount);

struct net_board* build_board(char *content, int length);

struct net_query* build_query(uint16_t cid, char *name, int length);

struct net_error* build_error(uint8_t ecode, char *detail, int length);

#endif
