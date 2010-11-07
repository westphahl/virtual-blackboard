#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"
#include "mq.h"

/*
 * Logger process
 *
 * Reads incoming log message from a message queue and
 * logs them to a file.
 * It takes the loglevel as a optional argument. By default
 * only error and info messages are logged.
 *
 * Loglevel:    1 = debug, info, error
 *              0 = info, error (default)
 */
int main(int argc, char **argv) {
    int mq_id;
    struct logmessage buffer;
    FILE* file;
    int debug = 0;

	/* Set debug mode */
    if (argc > 1) {
        debug = *argv[1];
    }

    /* Open logfile */
    if ((file = fopen("server.log", "w")) == NULL) {
        perror("fopen");
        exit(1);
    }

    /* Open the message queue */
    mq_id = get_mq(LOGGER_MQ_KEY);

    fprintf(stdout, "Logger: process started and waiting for log messages\n");

    /* Wait for new messages to arrive */
    while(1) {
        /* Get a new log message */
        buffer = read_mq(mq_id);

        /* Write log message to file */
        switch(buffer.level) {
        case 1:
        	/* Log error message */
            fprintf(file, "[ERROR] %ld %s \n", buffer.time, buffer.message);
            break;
        case 2:
        	/* Log info message */
            fprintf(file, "[INFO] %ld %s \n", buffer.time, buffer.message);
            break;
        case 3:
        	/* Log debug message */
            if(debug) {
                fprintf(file, "[DEBUG] %ld %s \n", buffer.time, buffer.message);
            }
            break;
        }
        
        fflush(file); // Flush logfile
    }
    fclose(file); // Close logfile
}
