#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "../net_message.h"

void* client_handler(void *sfd) {
    int socket_fd = * (int *) sfd;
    ssize_t ret;

    struct net_header *header = NULL;

    // Handle login of client
    if ((handle_login(socket_fd)) != 0) {
        // TODO Log error
        close(socket_fd);
        pthread_exit(1);
        return NULL;
    }

    while (1) {
        header = (struct net_header *) malloc(sizeof(struct net_header *));
        ret = recv(socket_fd, header, sizeof(struct net_header), MSG_WAITALL);
        ntoh_header(header);
        switch (header->type) {
            case m_login
        }
    }

    close(socket_fd);
    pthread_exit(0);
    return NULL;
}
