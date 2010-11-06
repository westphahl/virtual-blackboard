#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "shared.h"
#include "mq.h"
#include "utils.h"

/*
 * Send a log message via a message queue
 * It takes the log level as a required argument
 *
 * This function is not intended for direct usage.
 * Use a wrapper function instead.
 */
void log_message(int level, char* message) {
    int mq_id;
    struct logmessage buffer;
    /* Initialize buffer with 0 */
    memset(&buffer, 0, sizeof(struct logmessage));

    /* Get the message queue */
    mq_id = get_mq(LOGGER_MQ_KEY);

    buffer.type = MSGTYPE;
    buffer.level = level;
    /* Create timestamp */
    time(&buffer.time);

    strcpy(buffer.message, message);

    /* Send the log message */
    write_mq(mq_id, buffer);
}

/* Wrapper function for logging errors */
void log_error(char* message) {
    log_message(1, message);
}

/* Wrapper function for logging info messages */
void log_info(char* message) {
    log_message(2, message);
}

/* Wrapper function for logging debug messages */
void log_debug(char* message) {
    log_message(3, message);
}
