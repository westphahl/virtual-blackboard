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

struct cl_entry {
    struct client_data *cdata;
    struct cl_entry *next;
    struct cl_entry *previous;
};

void add_client(struct client_data *cdata);
int remove_client(int sfd);

int docent_exists();
int tutor_exists();
struct cl_entry* get_write_user(void);
int has_write_access(int sfd);

uint16_t get_next_cid(void);
int get_client_count(void);

struct cl_entry* start_iteration(void);
struct cl_entry* iteration_next(void);
void end_iteration(void);

#endif
