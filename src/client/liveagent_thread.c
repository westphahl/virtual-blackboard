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
#include "shared.h"
#include "utils.h"
#include "gui.h"
#include "../commons.h"

struct liveagentt_data *la_data;

void send_board_content(int sock) {
    int ret;
    int size;
	int gsize;
	char *buffer = get_blackboard();

    struct NET_MESSAGE_BOARD_CONTENT message;
    message.board_content = (char *)malloc((strlen(buffer)+1));
	if(message.board_content == NULL) {
		perror("malloc");
	}
    strcpy(message.board_content, buffer);

    size = strlen(message.board_content);
	gsize = sizeof(struct NET_HEADER) + strlen(message.board_content);

    message.head.type = (uint8_t) NET_TYPE_BOARD_CONTENT;
    message.head.lenght = htons((uint16_t) size);
    
	printf("Board-Content send. Size: %li\n", strlen(message.board_content));

    ret = 0;
    ret += write(sock, &message.head, sizeof(struct NET_HEADER));
    ret += write(sock, message.board_content, size);

    if(ret != gsize) {
        perror("write");
        printf("%d bytes\n", ret);
        fflush(stdout);
    }
	free(message.board_content);
}

void handler(int signum) {
	printf("Handler\n");
	fflush(stdout);
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

	while(1) {
		// TODO
	}
	
	pthread_exit(0);
    return NULL;
}
