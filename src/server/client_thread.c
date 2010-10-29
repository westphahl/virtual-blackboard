#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "../net_message.h"
#include "message_handler.h"

void* client_handler(void *sfd) {
    int socket_fd = * (int *) sfd;
    ssize_t ret;
    int f;

    struct net_header *header = NULL;

    // Handle login of client
    // TODO DEBUG
    fprintf(stdout, "Handle login ...\n");
    if ((f = login_handler(socket_fd)) != 0) {
        // TODO Log error
        fprintf(stdout, "Login failed: %i", f); 
        fflush(stdout);
        close(socket_fd);
        pthread_exit(NULL);
        return NULL;
    }
    fprintf(stdout, "Login success!\n");
    fflush(stdout);
/*
    while (1) {
        header = (struct net_header *) malloc(sizeof(struct net_header *));
        ret = recv(socket_fd, header, sizeof(struct net_header), MSG_WAITALL);
        ntoh_header(header);
        //switch (header->type) {
        //    case m_login
        //}
    }
*/

    close(socket_fd);
    pthread_exit(0);
    return NULL;
}
