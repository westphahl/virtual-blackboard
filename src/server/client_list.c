#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "client_list.h"
#include "../commons.h"


static struct cl_entry *cl_first = NULL;
static struct cl_entry *cl_last = NULL;
// Points to the user that is docent, if not available NULL
static struct cl_entry *user_docent = NULL;
// Points to the user who has write access
static struct cl_entry *user_write = NULL;

// Holds the current position of the iteration
static struct cl_entry *iterator = NULL;

/* Mutex for access to client list */
static pthread_mutex_t cl_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Counter for last client id and mutex */
static uint16_t cid_counter = 0;
static pthread_mutex_t cid_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Counter for number of active clients */
static int client_counter = 0;

/*
 * Add a new client to the client list.
 */
void add_client(struct client_data *cdata) {
    struct cl_entry *new_entry;

    // Allocate memory for new client list entry
    new_entry = (struct cl_entry *) malloc(sizeof(struct cl_entry));
    if (new_entry == NULL) {
        fprintf(stderr, "malloc: failed in client_list.c\n");
        exit(EXIT_FAILURE);
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

    if (new_entry->cdata->role == DOCENT) {
        user_docent = new_entry;
        user_write = new_entry;
    }

    client_counter++;
    pthread_mutex_unlock(&cl_mutex);
}

/*
 * Remove a client from the client list. This does NOT free
 * the client data.
 * Returns 0 on success or 1 if an error occured.
 */
int remove_client(int sfd) {
    struct cl_entry *old_entry;

    // Lock client list
    pthread_mutex_lock(&cl_mutex);

    for (old_entry = cl_first;
        (old_entry != NULL) && (old_entry->cdata->sfd != sfd);
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

    if (old_entry == user_write) {
        if (old_entry == user_docent) {
            user_write = NULL;
        } else {
            user_write = user_docent;
        }
    }

    if (old_entry == user_docent) {
        user_docent = NULL;
    }

    client_counter--;
    pthread_mutex_unlock(&cl_mutex);

    free(old_entry->cdata);
    free(old_entry);
    return 0;
}

/*
 * Searches the client list for a docent.
 * Returns 1 if a docent was found, otherwise 0.
 */
int docent_exists() {
    if (user_docent != NULL) {
        return 1;
    } else {
        return 0;
    }
}

/*
 * Check if there is a tutor
 * Returns 1 if there is one, otherwise 0.
 */
int tutor_exists() {
    if (user_docent == user_write) {
        return 0;
    } else {
        return 1;
    }
}

/*
 * Get the user who has write access
 */
struct cl_entry* get_write_user() {
    return user_write;
}

int has_write_access(int sfd) {
    int result;

    pthread_mutex_lock(&cl_mutex);
    if (user_write->cdata->sfd == sfd) {
        result = 1;
    } else {
        result = 0;
    }
    pthread_mutex_unlock(&cl_mutex);

    return result;
}

/*
 * Get the next available client id
 */
uint16_t get_next_cid(void) {
    uint16_t cid;
    pthread_mutex_lock(&cid_mutex);
    cid = ++cid_counter;
    pthread_mutex_unlock(&cid_mutex);
    return cid;
}

/*
 * Get the number of active clients
 */
int get_client_count(void) {
    return client_counter;
}

/*
 * Start the iteration
 * Make sure to call end_interation after finishing,
 * otherwise the client list is not unlocked.
 */
struct cl_entry* start_iteration() {
    pthread_mutex_lock(&cl_mutex);
    iterator = cl_first;
    return cl_first;
}

/*
 * Get the next entry from the list
 * Returns a entry of the client list or NULL if there
 * are no more entries in the list.
 */
struct cl_entry* iteration_next() {
    iterator = iterator->next;
    return iterator;
}

/*
 * End iteration
 */
void end_iteration() {
    iterator = NULL;
    pthread_mutex_unlock(&cl_mutex);
}
