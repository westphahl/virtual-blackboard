#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#include "../commons.h"
#include "signal_handler.h"

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
        execl("/bin/sleep", "sleep", "999", NULL);
    } else if (a_pid < 0) {
        perror("fork archiver");
        exit(EXIT_FAILURE);
    }
    
    // Don't care about exit status
    // Register signal handler
    signal(SIGINT, sigint);
    waitpid(l_pid, NULL, 0);
    waitpid(a_pid, NULL, 0);
    fprintf(stdout, "Child processes terminated.\n");

    exit(EXIT_SUCCESS);
}
