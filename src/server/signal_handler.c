/*
 * Module: signal_handler
 * Type: Function library
 * Description: Signal handlers for specific signal.
 */
#include <signal.h>
#include <stdio.h>
#include "signal_handler.h"

/*
 * Function: sigint()
 * Description: Handle SIGINT sent by CTRL-C so that the cleanup code
 * in the server process is executed.
 */
void sigint() {
    /* Reset signal */
    signal(SIGINT, sigint);
    fprintf(stdout, "\nWaiting for child processes to terminate ...\n");
}
