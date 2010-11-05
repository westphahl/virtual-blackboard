#define _POSIX_SOURCE

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

#include <gtk/gtk.h>

#include "client.h"
#include "liveagent_thread.h"
#include "command_thread.h"
#include "shared.h"
#include "utils.h"
#include "gui.h"
#include "../commons.h"
#include "../net_message.h"

static pthread_mutex_t trigger_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t trigger_lagent = PTHREAD_COND_INITIALIZER;
struct liveagentt_data *la_data;

void send_board_content() {
    int ret;
    int socket = la_data->socket;
	char *buffer = get_blackboard();

    struct net_board *msg;
    msg = (struct net_board *)alloc(sizeof(struct net_board)+strlen(buffer));
    memcpy(msg->content, buffer, strlen(buffer));

	msg->header.type = m_board;
	msg->header.length = htons(strlen(buffer));
    
	printf("Board-Content send. Size: %li, Socket: %i %i\n", sizeof(struct net_board)+strlen(buffer), la_data->socket, socket);

    ret = send(socket, msg, sizeof(struct net_board)+strlen(buffer), 0);

    if(ret < 0) {
        perror("send");
        fflush(stdout);
    }
    
	free(msg);
}

void handler(int signum) {
	printf("Handler\n");
	fflush(stdout);
}

void trigger_liveagent(int type) {
    pthread_mutex_lock(&trigger_mutex);
    pthread_cond_signal(&trigger_lagent);
    pthread_mutex_unlock(&trigger_mutex);
}

void *liveagent_handler(void *data) {
	la_data = (struct liveagentt_data *)data;
	
	printf("Sock: %i\n", la_data->socket);
	
    printf("Live-Agent gestartet\n");
	fflush(stdout);
	
	/* Install signal handler */
    struct sigaction sa;
	
    sa.sa_handler = send_board_content;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO; /* we want a siginfo_t */
    if (sigaction (SIGALRM, &sa, 0)) {
		perror("sigaction");
    }

	pthread_mutex_lock(&trigger_mutex);
    while(1) {
        /* Wait for the trigger */
        pthread_cond_wait(&trigger_lagent, &trigger_mutex);
		send_board_content(la_data->socket);
    }
    pthread_mutex_unlock(&trigger_mutex);
	
	pthread_exit(0);
    return NULL;
}
