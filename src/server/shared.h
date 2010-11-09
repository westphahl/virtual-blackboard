#ifndef SHARED_H
#define SHARED_h

/*
 * Data shared between server, logger and archiver.
 */

/* Pathr for ftok() */
#define FTOK_PATH "/dev/null"

/* Message queue: id for ftok() */
#define LMQ_ID 1

/* Shared memory: id for ftok() */
#define BSHM_ID 1

/* Semaphores: ids for ftok() */
#define BSEM_ID 1
#define ASEM_ID 2

#endif
