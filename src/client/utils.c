// System headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// User header
#include "../net_message.h"

static pthread_mutex_t board_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t cdata_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Helper function to allocate disk space.
 */
void *alloc(size_t size) {
	/* allocate disk space */
	void *tmp = malloc(size);
	if(tmp == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	
	/* set all to NULL */
	memset(tmp, 0, size);
	
	return tmp;
}

/*
 * Lock the board mutex
 */
void board_lock() {
	pthread_mutex_lock(&board_mutex);
}

/*
 * Unlock the board mutex
 */
void board_unlock() {
	pthread_mutex_unlock(&board_mutex);
}

/*
 * Destroy the board mutex
 */
void board_destroy() {
	pthread_mutex_destroy(&board_mutex);
}

/*
 * Lock the client-data mutex
 */
void cdata_lock() {
	pthread_mutex_lock(&cdata_mutex);
}

/*
 * Unlock the client-data mutex
 */
void cdata_unlock() {
	pthread_mutex_unlock(&cdata_mutex);
}

/*
 * Destory the client-data mutex
 */
void cdata_destroy() {
	pthread_mutex_destroy(&cdata_mutex);
}
