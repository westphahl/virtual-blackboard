#define _POSIX_SOURCE 1

#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "../commons.h"
#include "shared.h"
#include "signal_handler.h"
#include "mq.h"
#include "blackboard.h"
#include "login_thread.h"

/*
 * Server for the virtual blackboard.
 *
 * Takes two optional arguments:
 *      -d          Run server in debug mode
 *      -p <PORT>   Use a user-defined port
 */
int main(int argc, char **argv) {
    unsigned int lport = strtol(DEFAULT_PORT, NULL , 10); // Server TCP port
    char *listen_port = DEFAULT_PORT; // Server TCP port as string
    int debug = 0; // Debug mode; default off
    int opt;

    pid_t l_pid; // Holds the pid of the logger
    pid_t a_pid; // Holds the pid of the archiver

    key_t lmq_key = LOGGER_MQ_KEY; // Key for logger message queue
    int lmq_id; // Id of the message queue for loggin

    key_t bshm_key = BLACKBOARD_SHM_KEY; // Key for the blackboard
    int bshm_id; // Id of the shared memory segment for the blackboard

    struct addrinfo hints; // Hints for getaddrinfo()
    struct addrinfo *result, *rp; // Holds result of getaddrinfo()
    int sfd; // Temporary socket file descriptor
    int socket_fds[125]; // File descriptors of sockets
    int socket_count = 0; // Number of socket fds
    int ret; // Return value of getaddrinfo()

    struct logint_data lt_data; // Pointer to login thread data
    pthread_t login_tid; // Id of the login thread

    /*
     * Parse command line option
     * User defined port:   -p <PORT-NR>    (optional)
     * Debug mode:          -d              (optional)
     */
    while ((opt = getopt(argc, argv, "dp:")) != -1) {
        switch(opt) {
            case 'd':
                // Run server in debug mode
                debug = 1;
                break;
            case 'p':
                /* 
                 * Use a user-defined port
                 * The port is converted to int for port range comparison
                 */
                lport = strtol(optarg, NULL, 10);
                if ((lport < PORT_RANGE_MIN) || (lport > PORT_RANGE_MAX)) {
                    fprintf(stderr,
                        "Invalid port range: must be between %i and %i.\n" \
                        "Falling back to default: %s\n",
                        PORT_RANGE_MIN, PORT_RANGE_MAX, DEFAULT_PORT);
                } else {
                    listen_port = optarg;
                }
                break;
        }
    }

    /*
     * Register signal handler, so CTRL-C does not kill the server
     * and the cleanup code is executed;
     */
    signal(SIGINT, sigint);

    /*
     * Bind to all available sockets IPv4 + IPv6
     */

    // Set all bits of hint to 0
    memset(&hints, 0, sizeof(struct addrinfo));

    // Prepare hints for getaddrinfo()
    hints.ai_family = AF_UNSPEC; // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Stream socket
    hints.ai_protocol = IPPROTO_TCP; // Use TCP
    hints.ai_flags = AI_PASSIVE;// | AI_V4MAPPED;

    ret = getaddrinfo(NULL, listen_port, &hints, &result);
    if (ret != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
    }

    /* 
     * Try to bind() all adress structures returned by getaddrinfo().
     * If socket() or bind() fails, close the socket and try
     * the next address.
     */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        int on = 1;

        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            perror("socket");
            continue;
        }

        //Reuse the socket
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            perror("setsockopt"); // Maybe not so fatal, continue ...
        }

        // Set IPv6 options
        if (rp->ai_family == AF_INET6) {
            if (setsockopt(sfd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) < 0) {
                perror("setsockopt");
            }
        }

        // TODO Debug output
        char dst[INET6_ADDRSTRLEN];
        char service[INET6_ADDRSTRLEN];
        getnameinfo(rp->ai_addr,
                rp->ai_addrlen,
                dst,
                sizeof(dst),
                service,
                sizeof(service),
                NI_NUMERICHOST | NI_NUMERICSERV);

        fprintf(stdout, "Trying %s:%s ...\n", dst, service),
        fflush(stdout);
        // TODO END

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            if (listen(sfd, 5) == -1) {
                perror("listen");
                close(sfd);
            } else {
                // Add to valid sockets
                socket_fds[socket_count++] = sfd;
            }
        } else {
            perror("bind");
            close(sfd);
        }
        // TODO Debug output
        fprintf(stdout, "Socket: %i, %i\n", sfd, socket_count);
    }

    // We no longer need all the address structures
    freeaddrinfo(result);
    
    // Exit if there are no sockets to bind to
    if (socket_count == 0) {
        fprintf(stderr, "No sockets to bind to!\n");
        exit(EXIT_FAILURE);
    }

    // Create message queue for logger
    lmq_id = create_mq(lmq_key);

    // Create blackboard in shared memory
    bshm_id = init_blackboard(bshm_key);

    // Fork Logger
    l_pid = fork();
    if (l_pid == 0) {
        // TODO Exec logger
        execl("/bin/sleep", "sleep", "999", NULL);
    } else if (l_pid < 0) {
        perror("fork logger");
        exit(EXIT_FAILURE);
    }
    
    // Fork Archiver
    a_pid = fork();
    if (a_pid == 0) {
        // TODO Exec archiver
        // Pass debug as a command line argument.
        execl("/bin/sleep", "sleep", "999", NULL);
    } else if (a_pid < 0) {
        perror("fork archiver");
        exit(EXIT_FAILURE);
    }

    /*
     * Prepare and create login thread accept loop
     */
    lt_data.fds = socket_fds;
    lt_data.fd_count = socket_count;
    pthread_create(&login_tid, NULL, login_handler, (void *) &lt_data);

    /*
     * Just wait for the logger and archiver to terminate
     * We don't care about exit status
     */
    waitpid(l_pid, NULL, 0);
    waitpid(a_pid, NULL, 0);
    fprintf(stdout, "Child processes terminated.\n");

    // Close all sockets
    for (int i = 0; i < socket_count; i++)
        close(socket_fds[i]);

    // Delete shared memory segment
    delete_blackboard(bshm_id);
    // Delete message queue
    delete_mq(lmq_id);

    exit(EXIT_SUCCESS);
}
