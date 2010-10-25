/*
 * stdlib.h: 
 *      exit()
 *      EXIT_SUCCESS, EXIT_FAILURE
 * stdio.h:
 *      fprintf()
 *      stdout
 * getopt.h:
 *      getopt()
 *      optarg
 */
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../commons.h"

int main(int argc, char **argv) {
    unsigned int listen_port = DEFAULT_PORT;
    int debug = 0;
    int opt;
    pid_t l_pid;
    pid_t a_pid;

    // Parse command line options
    while ((opt = getopt(argc, argv, "dp:")) != -1) {
        switch(opt) {
            case 'd':
                // Run server in debug mode
                debug = 1;
                break;
            case 'p':
                // Use a user defined port
                listen_port = strtol(optarg, NULL, 10);
                if ((listen_port < PORT_RANGE_MIN) || (listen_port > PORT_RANGE_MAX)) {
                    fprintf(stderr,
                        "Invalid port range: must be between %i and %i.\n" \
                        "Falling back to default %i\n",
                        PORT_RANGE_MIN, PORT_RANGE_MAX, DEFAULT_PORT);
                    listen_port = DEFAULT_PORT;
                }
                break;
        }
    }

    // Fork Logger
    l_pid = fork();
    if (l_pid == 0) {
        // Logger: excel()
    } else if (l_pid < 0) {
        perror("fork logger");
        exit(EXIT_FAILURE);
    } else {
        // Fork Archiver
        a_pid = fork();
        if (a_pid == 0) {
            // Archiver: excel()
        } else if (a_pid < 0) {
            perror("fork archiver");
            exit(EXIT_FAILURE);
        } else {
            // Server goes here
        }
    }

    exit(EXIT_SUCCESS);
}
