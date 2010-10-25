#include <signal.h>
#include <stdio.h>
#include "signal_handler.h"

void sigint() {
    // Reset signal
    signal(SIGINT, sigint);
    fprintf(stdout, "\nWaiting for child processes to terminate ...\n");
}
