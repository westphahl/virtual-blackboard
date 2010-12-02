#define _POSIX_SOURCE

// System headers
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

// GTK header
#include <gtk/gtk.h>

// User headers
#include "liveagent_thread.h"
#include "client.h"
#include "command_thread.h"
#include "shared.h"
#include "utils.h"
#include "../commons.h"
#include "../net_message.h"

// Mutex
static pthread_mutex_t la_data_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t trigger_la_mutex = PTHREAD_MUTEX_INITIALIZER;
// Condition variable
static pthread_cond_t trigger_lagent = PTHREAD_COND_INITIALIZER;
// Thread data
struct liveagentt_data *la_data;

/*
 * Send board content to server.
 */
void send_board_content() {
    int ret = 0;
    pthread_mutex_lock(&la_data_mutex); // Lock la_data mutex
    int socket = la_data->socket;
    pthread_mutex_unlock(&la_data_mutex); // Unlock la_data mutex
    
    // Get blackboard from client
    board_lock();
	char *buffer = get_blackboard();

	// Allocate disk space for board message
    struct net_board *msg = (struct net_board *)alloc(sizeof(struct net_board)+strlen(buffer));
    
    // Fill message with data
    memcpy(msg->content, buffer, strlen(buffer));
	msg->header.type = m_board;
	msg->header.length = htons(strlen(buffer));
    
	printf("Board-Content send. Size: %li\n", sizeof(struct net_board)+strlen(buffer));

	// Send message to server
    ret = send(socket, msg, sizeof(struct net_board)+strlen(buffer), 0);

    if(ret < 0) {
        perror("send");
        fflush(stdout);
    }
    
    board_unlock();

    // Release allocated disk space
	free(msg);
}

/*
 * Trigger to start live-agent.
 */
void trigger_liveagent() {
    pthread_mutex_lock(&trigger_la_mutex); // Lock la mutex
    pthread_cond_signal(&trigger_lagent); // Set condition signal
    pthread_mutex_unlock(&trigger_la_mutex); // Unlock la mutex
}

/*
 * Live-agent to watch blackboard and send it to server.
 */
void *liveagent_handler(void *data) {
	pthread_mutex_lock(&la_data_mutex); // Lock la_data mutex
	la_data = (struct liveagentt_data *)data;
	pthread_mutex_unlock(&la_data_mutex); // Unlock la_data mutex
	
    printf("Live-Agent gestartet\n");
	fflush(stdout);
	
	// Setup signal handler
    struct sigaction sa;
    sa.sa_handler = send_board_content; // Set function to call
    sigemptyset(&sa.sa_mask); // Set mask to null
    sa.sa_flags = SA_SIGINFO; // Set signal flag
    if (sigaction (SIGALRM, &sa, 0)) {
		perror("sigaction");
    }

	pthread_mutex_lock(&trigger_la_mutex); // Lock la mutex
    while(1) {
        // Wait for the trigger
        pthread_cond_wait(&trigger_lagent, &trigger_la_mutex);
		send_board_content(la_data->socket); // Send board
    }
    pthread_mutex_unlock(&trigger_la_mutex); // Unlock la mutex
	
	pthread_exit(0);
    return NULL;
}
