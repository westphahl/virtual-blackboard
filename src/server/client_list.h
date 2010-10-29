#ifndef CLIENT_LIST_H
#define CLIENT_LIST_H

#include <stdint.h>
#include <pthread.h>

#pragma pack(1)
struct client_data {
    uint16_t cid;
    int sfd;
    uint8_t role;
    char *name[];
};
#pragma pack(0)

int add_client(struct client_data *cdata);

int remvove_client(struct client_data *cdata);

#endif
