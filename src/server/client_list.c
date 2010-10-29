#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "client_list.h"

struct cl_entry {
    struct client_data *cdata;
    struct cl_entry *next;
    struct cl_entry *previous;
};

static struct cl_entry *cl_first = NULL;
static struct cl_entry *cl_last = NULL;

static pthread_mutex_t cl_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Add a new client to the client list.
 * Returns 0 on success or 1 if malloc fails.
 */
int add_client(struct client_data *cdata) {
    struct cl_entry *new_entry;

    // Allocate memory for new client list entry
    new_entry = (struct cl_entry *) malloc(sizeof(struct cl_entry));
    if (new_entry == NULL) {
        fprintf(stderr, "malloc: failed in client_list.c\n");
        return 1;
    }

    new_entry->cdata = cdata;
    
    // Lock the client list
    pthread_mutex_lock(&cl_mutex);

    // See if list is empty
    if (cl_first == NULL) {
        cl_first = new_entry;
    } else {
        cl_last->next = new_entry;
    }
    
    // Set pointers appropriately
    new_entry->previous = cl_last;
    new_entry->next = NULL;

    // Set new last element
    cl_last = new_entry;

    pthread_mutex_unlock(&cl_mutex);

    return 0;
}

/*
 * Remove a client from the client list. This does NOT free
 * the client data.
 * Returns 0 on success or 1 if an error occured.
 */
int remove_client(struct client_data *cdata) {
    struct cl_entry *old_entry;

    // Lock client list
    pthread_mutex_lock(&cl_mutex);

    for (old_entry = cl_first;
        (old_entry == NULL) && (old_entry->cdata == cdata);
         old_entry = old_entry->next)

    if (old_entry == NULL) return 1;
    
    // First element of client list?
    if (old_entry->previous == NULL) {
        cl_first = old_entry->next;
    } else {
        old_entry->previous->next = old_entry->next;
    }

    // Last element of client list?
    if (old_entry->next == NULL) {
        cl_last = old_entry->previous;
    } else {
        old_entry->next->previous = old_entry->previous;
    }

    pthread_mutex_unlock(&cl_mutex);

    free(old_entry);
    return 0;
}
