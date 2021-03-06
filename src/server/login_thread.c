#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#include "login_thread.h"
#include "client_thread.h"
#include "utils.h"

/*
 * Accept loop for new connection attempts.
 * For every new connection a new client thread
 * is created to handle the connection.
 */
void* login_thread(void *data) {
    struct logint_data *lt_data = (struct logint_data *) data;
    fd_set set; // File descriptor set
    int *socket_fds = lt_data->fds; // Array of socket fds
    int socket_count = lt_data->fd_count; // Number of sockets
    int highest_fd; // Highest-numbered file descriptor
    int ready; // Return value of select() (number of ready connections)
    int accept_fd; // Socket fd returned by accept()
    pthread_t client_tid; // Id of the client_thread

    while(1) {
        /* Highest numbered fd which is needed for select() */
        highest_fd = 0;
        /* Clear out the set for select() */
        FD_ZERO(&set);
        
        /*
         * Get the highest-numbered socket file descriptor
         * from the array of socket fds (needed for select()), 
         * and add all to the file descriptor set.
         */
        for (int i = 0; i < socket_count; i++) {
            if (socket_fds[i] > highest_fd) {
                highest_fd = socket_fds[i];
            }
            FD_SET(socket_fds[i], &set);
        }

        /* struct logint_data *lt_data = (struct logint_data *) data;
         * Wait for one or more socket fds to become "ready"
         * select() returns the number of file descriptors 
         * contained in the descriptor set
         *
         * If select() was interrupted by a signal the select loop
         * is restarted.
         */
        ready = select(highest_fd+1, &set, NULL, NULL, NULL);
        if (ready == -1) {
            if (errno != EINTR) {
                perror("select");
                return NULL;
            } else {
                /* select() was interrupted by a signal */
                continue;
            }
        }

        for (int i = 0; i < socket_count; i++) {
            /* Check if file descriptor is set in the fd set */
            if (FD_ISSET(socket_fds[i], &set)) {
                /* Accept the new connection */
                accept_fd = accept(socket_fds[i], NULL, NULL);
                if (accept_fd < 0) {
                    perror("accept");
                } else {
                    /*
                     * Create a new client Thread
                     */
                    pthread_create(&client_tid, NULL, 
                            client_handler, (void *) (&accept_fd));
                    pthread_detach(client_tid);
                    log_info("Created client thread for a new connection");
                }
            }
        }
    }
    pthread_exit(0);
    return NULL;
}
