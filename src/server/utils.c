#define _XOPEN_SOURCE

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

void log_message(int level, char* message) {
    int mq_id;
    // TODO Initialize buffer
    // Run valgrind to see this error.
    struct logmessage buffer;
    // Initialize buffer
    memset(&buffer, 0, sizeof(struct logmessage));

    mq_id = get_mq(LOGGER_MQ_KEY);

    buffer.type = MSGTYPE;
    buffer.level = level;
    time(&buffer.time);

    strcpy(buffer.message, message);

    write_mq(mq_id, buffer);
    printf("Message \"%s\" sent\n", buffer.message);
}

void log_error(char* message) {
    log_message(1, message);
}

void log_info(char* message) {
    log_message(2, message);
}

void log_debug(char* message) {
    log_message(3, message);
}
