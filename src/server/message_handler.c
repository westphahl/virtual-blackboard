#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../net_message.h"
#include "message_handler.h"
#include "client_list.h"

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

    // TODO Debug
    fprintf(stdout, "Waiting for login ...\n");
    /*
     * Wait for client login
     */
    header = (struct net_header *) malloc(sizeof(struct net_header));
    ret = recv(sfd, header, sizeof(struct net_header), MSG_WAITALL);

    // TODO Debug
    fprintf(stdout, "Got message type: %i; required: %i\n", 
            header->type, m_login);
    fprintf(stdout, "Message size: %i; struct: %i\n", ret,
            sizeof(struct net_header));
    // Close socket if not a login message
    if (header->type != m_login) {
        fprintf(stdout, "Wrong mtype!\b");
        fflush(stdout);
        return 1;
    }

    // Connection closed by client or error
    if (ret <= 0) {
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
        return 1;
    }

    // Allocate memory for receive buffer
    rbuf = malloc(header->length);
    // Receive login message
    ret = recv(sfd, rbuf, header->length, MSG_WAITALL);

    //Connection closed by client or error
    if (ret <= 0) {
        free(header);
        free(rbuf);
        return -1;
    }

    // TODO Debug
    fprintf(stdout, "Got message of size: %i\n", ret);

    // Allocate memory for client login message
    cdata = (struct client_data *) malloc(sizeof(struct client_data));

    // TODO DEBUG
    fprintf(stdout, "--- OK malloc: cdata---\n");

    // TODO Generate CID
    cdata->cid = 123;
    cdata->sfd = sfd;
    // Read role
    // TODO ERROR WARNING MEMORY CORRUPTION
    fprintf(stdout, "memcpy: pointer: %p offset: %i size: %i \nnew pointer: %p\n", 
            cdata, (sizeof(uint16_t) + sizeof(int)), sizeof(uint8_t),
            (cdata + sizeof(uint16_t) + sizeof(int)));
    //memcpy(cdata + sizeof(uint16_t) + sizeof(int), rbuf, sizeof(uint8_t));

    fprintf(stdout, "--- OK memcpy: cdata---\n");

    // Read name
    //cname = (char *) malloc(header->length - sizeof(uint8_t));
    fprintf(stdout, "malloc size: %i\n", (header->length - sizeof(uint8_t)));

    //////////////
    free(header);
    free(rbuf);
    free(cdata);
    return -1;
    //////////////

/*
    // TODO DEBUG
    fprintf(stdout, "--- OK --- malloc: cname\n");
    
    memcpy(cdata + sizeof(uint16_t) + sizeof(int) + sizeof(uint8_t),
            cname, header->length - sizeof(uint8_t));

    fprintf(stdout, "--- OK memcpy: cname ---\n");

    switch (cdata->role) {
        case 0:
            // TODO Check if docent existes
            // If not make user docent
            cdata->role = 1;
            break;
        case 1:
            break;
        case 2:
            // TODO Check if docent exists
            // If not make user student
            break;
        default:
            // TODO Non-RFC compliant: log error
            cdata->role = 1;
    }

    add_client(cdata);
*/
    return 0;
}
