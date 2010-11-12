// System headers
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
#include <sys/msg.h>

// GTK header
#include <gtk/gtk.h>

// User headers
#include "command_thread.h"
#include "client.h"
#include "gui.h"
#include "listener_thread.h"
#include "utils.h"
#include "shared.h"
#include "../commons.h"
#include "../net_message.h"

// Mutex for condition variable
static pthread_mutex_t trigger_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;
// Condition variable
static pthread_cond_t trigger_cmd = PTHREAD_COND_INITIALIZER;
static int command_type = m_login;
// Various data structs
static struct commandt_data *cm_data;

/*
 * Send login-data to server
 */
void send_login(int socket, uint8_t role, char *name) {
    int ret;

	// Allocate disk space for message
	struct net_login *msg;
    msg = (struct net_login *)alloc(sizeof(struct net_login)+strlen(name));
    
    // Fill message with data
    msg->role = role;
    memcpy(msg->name, name, strlen(name));

	msg->header.type = m_login;
	msg->header.length = htons(sizeof(uint8_t) + strlen(name));
    
	printf("Login send. Size: %li\n", sizeof(msg));

	// Send message to server
    ret = send(socket, msg, sizeof(struct net_login)+strlen(name), 0);

    if(ret < 0) {
        perror("send");
        fflush(stdout);
    }
    
    // Release allocated disk space
	free(msg);
}

/*
 * Send command to clean board.
 */
void send_board_clean(int socket) {
    int ret;
    
	// Allocate disk space for message
	struct net_header *msg = (struct net_header *)alloc(sizeof(struct net_header));

	// Fill message with data
	msg->type = m_clear;
	msg->length = 0;
    
	printf("Board-Clear send. Size: %li\n", sizeof(msg));

	// Send message to server
    ret = send(socket, msg, sizeof(struct net_header), 0);

    if(ret < 0) {
        perror("send");
        fflush(stdout);
    }
    
    // Release allocated disk space
	free(msg);
}

/*
 * Send command to request or release write permission.
 */
void send_request(int socket, uint8_t write) {
    int ret;
    
    // Allocate disk space for message
    struct net_request *msg;
    msg = (struct net_request *)alloc(sizeof(struct net_request));

	// Fill message with data
	msg->header.type = m_request;
	msg->header.length = htons(sizeof(struct net_request)-sizeof(struct net_header));
	
	msg->write = write;
    
	printf("Request send. Size: %li\n", sizeof(msg));

	// Send message to server
    ret = send(socket, msg, sizeof(struct net_request), 0);

    if(ret < 0) {
        perror("send");
        fflush(stdout);
    }
    
    // Release allocated disk space
	free(msg);
}

/*
 * Send answer if someone require write permission.
 */ 
void send_reply(int socket, uint8_t write, uint16_t cid) {
    int ret;
    
    // Allocate disk space for message
    struct net_reply *msg;
    msg = (struct net_reply *)alloc(sizeof(struct net_reply));

	// Fill message with data
	msg->header.type = m_reply;
	msg->header.length = htons(sizeof(struct net_reply)-sizeof(struct net_header));
	
	msg->write = write;
	msg->cid = htons(cid);
    
	printf("Reply send. Size: %li\n", sizeof(*msg));

	// Send message to server
    ret = send(socket, msg, sizeof(struct net_reply), 0);

    if(ret < 0) {
        perror("send");
        fflush(stdout);
    }
    
    // Release allocated disk space
	free(msg);
}

/*
 * Send shutdown-command to server.
 */
void send_shutdown(int socket) {
    int ret;
    
    // Allocate disk space for message
    struct net_header *msg;
    msg = (struct net_header *)alloc(sizeof(struct net_header));

	// Fill message with data
	msg->type = m_shutdown;
	msg->length = 0;
    
	printf("Shutdown send. Size: %li\n", sizeof(*msg));

	// Send message to sever
    ret = send(socket, msg, sizeof(struct net_header), 0);

    if(ret < 0) {
        perror("send");
        fflush(stdout);
    }
    
    // Release allocated disk space
	free(msg);
}

/*
 * Trigger to call commands.
 */
void trigger_command(int type) {
    pthread_mutex_lock(&trigger_cmd_mutex); // Lock cmd mutex
    command_type = type; // Set command type
    pthread_cond_signal(&trigger_cmd); // Send condition signal
    pthread_mutex_unlock(&trigger_cmd_mutex); // Unlock cmd mutex
}

/*
 * Command-thread to send commands to the server.
 */
void *command_handler(void *data) {
	cm_data = (struct commandt_data *)data;
	int socket = cm_data->socket;
	struct client_data *cdata = cm_data->cdata;

    printf("Command-Thread gestartet\n");
	fflush(stdout);
	
	cdata_lock();
    send_login(socket, cdata->role, cdata->name);
    cdata_unlock();
	
	pthread_mutex_lock(&trigger_cmd_mutex); // Lock cmd mutex
    while(1) {
        // Wait for trigger
        pthread_cond_wait(&trigger_cmd, &trigger_cmd_mutex);
        switch (command_type) {
            case m_login:
            	// Send login
            	printf("Login senden ...\n");
            	fflush(stdout);
            	cdata_lock();
                send_login(socket, cdata->role, cdata->name);
                cdata_unlock();
                break;
            case m_clear:
            	// Send clean board
            	printf("Clear senden ...\n");
            	fflush(stdout);
            	send_board_clean(socket);
                break;
            case m_request:
            	// Send request
            	printf("Request senden ...\n");
            	fflush(stdout);            	
            	if(cdata->role == 2) {
            		send_request(socket, 1);
            	} else {
            		send_request(socket, !cdata->write);
            	}
                break;
            case m_shutdown:
            	// Send reply
            	printf("Shutdown senden ...\n");
            	fflush(stdout);
            	send_shutdown(socket);
                break;
        }
    }
    pthread_mutex_unlock(&trigger_cmd_mutex); // Unlock cmd mutex
	
	pthread_exit(0);
    return NULL;
}
