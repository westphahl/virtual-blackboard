#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sem.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <gtk/gtk.h>

#include "command_thread.h"
#include "client.h"
#include "shared.h"
#include "utils.h"
#include "../commons.h"
#include "../net_message.h"

static pthread_mutex_t trigger_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t trigger_cmd = PTHREAD_COND_INITIALIZER;
static int command_type = m_login;
static struct net_reply *rdata;
static struct commandt_data *cm_data;

void send_login(int socket, uint8_t role, char *name) {
    int ret;

	struct net_login *msg;
    msg = (struct net_login *)alloc(sizeof(struct net_login)+strlen(name));
    msg->role = role;
    memcpy(msg->name, name, strlen(name));

	msg->header.type = m_login;
	msg->header.length = htons(sizeof(uint8_t) + strlen(name));
    
	printf("Login send. Size: %li\n", sizeof(msg));

    ret = send(socket, msg, sizeof(struct net_login)+strlen(name), 0);

    if(ret < 0) {
        perror("write");
        fflush(stdout);
    }
    
	free(msg);
}

void send_board_clean(int socket) {
    int ret;

	struct net_header *msg;
    msg = (struct net_header *)alloc(sizeof(struct net_header));

	msg->type = m_clear;
	msg->length = 0;
    
	printf("Board-Clear send. Size: %li\n", sizeof(msg));

    ret = send(socket, msg, sizeof(struct net_header), 0);

    if(ret < 0) {
        perror("write");
        fflush(stdout);
    }
    
	free(msg);
}

void send_request(int socket, uint8_t write) {
    int ret;
    
    struct net_request *msg;
    msg = (struct net_request *)alloc(sizeof(struct net_request));

	msg->header.type = m_request;
	msg->header.length = htons(sizeof(struct net_request)-sizeof(struct net_header));
	
	msg->write = write;
    
	printf("Request send. Size: %li\n", sizeof(msg));

    ret = send(socket, msg, sizeof(struct net_request), 0);

    if(ret < 0) {
        perror("write");
        fflush(stdout);
    }
    
	free(msg);
}

void send_reply(int socket, uint8_t write, uint16_t cid) {
    int ret;
    
    struct net_reply *msg;
    msg = (struct net_reply *)alloc(sizeof(struct net_reply));

	msg->header.type = m_reply;
	msg->header.length = htons(sizeof(struct net_reply)-sizeof(struct net_header));
	
	msg->write = write;
	msg->cid = cid;
    
	printf("Reply send. Size: %li\n", sizeof(*msg));

    ret = send(socket, msg, sizeof(struct net_reply), 0);

    if(ret < 0) {
        perror("write");
        fflush(stdout);
    }
    
	free(msg);
}

void send_shutdown(int socket) {
    int ret;
    
    struct net_header *msg;
    msg = (struct net_header *)alloc(sizeof(struct net_header));

	msg->type = m_shutdown;
	msg->length = 0;
    
	printf("Shutdown send. Size: %li\n", sizeof(*msg));

    ret = send(socket, msg, sizeof(struct net_header), 0);

    if(ret < 0) {
        perror("write");
        fflush(stdout);
    }
    
	free(msg);
}

void trigger_command(int type, struct net_reply *reply) {
	printf("Trigger aufgerufen. Type: %i\n", type);
	fflush(stdout);
    pthread_mutex_lock(&trigger_cmd_mutex);
    command_type = type;
    rdata = reply;
    pthread_cond_signal(&trigger_cmd);
    pthread_mutex_unlock(&trigger_cmd_mutex);
    printf("Trigger ende\n");
    fflush(stdout);
}

void *command_handler(void *data) {
	cm_data = (struct commandt_data *)data;
	int socket = cm_data->socket;
	struct client_data *cdata = cm_data->cdata;
	
	printf("Sock: %i\n", socket);
	
	send_login(socket, cdata->role, cdata->name);
	
    printf("Command-Thread gestartet\n");
	fflush(stdout);
	
	pthread_mutex_lock(&trigger_cmd_mutex);
    while(1) {
        /* Wait for the trigger */
        printf("while ... ");
        fflush(stdout);
        pthread_cond_wait(&trigger_cmd, &trigger_cmd_mutex);
        printf("Type: %i\n", command_type);
        fflush(stdout);
        switch (command_type) {
            case m_login:
            	printf("Login senden ...\n");
            	fflush(stdout);
                send_login(socket, cdata->role, cdata->name);
                break;
            case m_clear:
            	printf("Clear senden ...\n");
            	fflush(stdout);
            	send_board_clean(socket);
                break;
            case m_request:
            	printf("Request senden ...\n");
            	fflush(stdout);
            	send_request(socket, !cdata->write);
                break;
            case m_reply:
            	printf("Reply senden ...\n");
            	fflush(stdout);
            	send_reply(socket, rdata->write, rdata->cid);
            	rdata =  NULL;
                break;
            case m_shutdown:
            	printf("Shutdown senden ...\n");
            	fflush(stdout);
            	send_shutdown(socket);
                break;
        }
    }
    pthread_mutex_unlock(&trigger_cmd_mutex);
	
	pthread_exit(0);
    return NULL;
}
