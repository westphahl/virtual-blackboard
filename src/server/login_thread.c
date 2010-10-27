#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "login_thread.h"
#include "client_thread.h"

/*
 * Accept loop for new connection attempts.
 * For every new connection a new client thread
 * is created to handle the connection.
 */
void* login_handler(void *data) {
    fd_set set; // File descriptor set
    struct logint_data *lt_data = (struct logint_data *) data;
    int *socket_fds = lt_data->fds;
    int socket_count = lt_data->fd_count;
    int highest_fd; // Highest-numbered file descriptor
    int ready;
    int accept_fd; // Socket fd returned by accept()
    pthread_t client_tid; // Id of the client_thread

    while(1) {
        highest_fd = 0;
        FD_ZERO(&set);
        
        /*
         * Get the highest-numbered socket file descriptor
         * from the array of socket fds, and add all to 
         * the file descriptor set.
         */
        for (int i = 0; i < socket_count; i++) {
            if (socket_fds[i] > highest_fd) {
                highest_fd = socket_fds[i];
            }
            FD_SET(socket_fds[i], &set);
        }

        /*
         * Wait for one or more socket fd to become "ready"
         * select() returns the number of file descriptors 
         * contained in the descriptor set
         */
        ready = select(highest_fd+1, &set, NULL, NULL, NULL);
        if (ready == -1) {
            perror("select");
            return NULL;
        }

        for (int i = 0; i < socket_count; i++) {
            // Check if file descriptor is set in the fd set
            if (FD_ISSET(socket_fds[i], &set)) {
                // Accept the new connection
                accept_fd = accept(socket_fds[i], NULL, NULL);
                if (accept_fd < 0) {
                    perror("accept");
                } else {
                    fprintf(stdout, "A new connection!\nDeligate ...\n");

                    /*
                     * Create a new client Thread
                     */
                    pthread_create(&client_tid, NULL, 
                            client_handler, (void *) (&accept_fd));
                }
            }
        }
    }
    pthread_exit(0);
    return NULL;
}
