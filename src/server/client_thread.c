#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "../net_message.h"
#include "message_handler.h"
#include "utils.h"
#include "client_list.h"
#include "broadcasting.h"

void* client_handler(void *sfd) {
    int socket_fd = * (int *) sfd;
    int ret;
    struct net_header *header;

    // Handle login of client
    log_info("client thread: handle a new login");

    if ((ret = login_handler(socket_fd)) != 0) {
        log_error("client thread: connection closed by client");
        close(socket_fd);
        pthread_exit(NULL);
    }

    log_info("client thread: login successful");

    while (1) {
        header = (struct net_header *) malloc(sizeof(struct net_header));
        ret = recv(socket_fd, header, sizeof(struct net_header), MSG_WAITALL);

        // Connection was closed by client
        if (ret == 0) {
            log_info("client thread: connection closed by client");
            break;
        }

        ntoh_header(header);
        switch (header->type) {
            case m_board:
                ret = board_handler(socket_fd, header->length);
                break;
            default:
                // Non-RFC compliant message received
                // TODO Set to -1
                ret = 0;
        }
        free(header);

        if (ret < 0) {
            break;
        }
    }

    ret = remove_client(socket_fd);
    trigger_status();
    close(socket_fd);
    pthread_exit(NULL);
}
