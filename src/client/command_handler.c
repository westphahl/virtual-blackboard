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

#include "command_handler.h"
#include "shared.h"
#include "../commons.h"

void prepare_message(void *hdr, int type, int lenght) {
    struct NET_HEADER *phdr = (struct NET_HEADER *)hdr;
    
    phdr->type = (uint8_t) type;
    phdr->lenght = htons((uint16_t) lenght);
}

void send_login(int sock, char *name, uint8_t role) {
    int ret;
    int size;
	int gsize;

    struct NET_MESSAGE_LOGIN message;
    message.role = role;
    message.name = name;

    size = sizeof(uint8_t) + strlen(message.name);
	gsize = sizeof(struct NET_HEADER) + sizeof(uint8_t) + strlen(message.name);

    prepare_message(&message, NET_TYPE_LOGIN, size);
    
	printf("Login send. Size: %i\n", gsize);

    ret = 0;
    ret += write(sock, &message.head, sizeof(struct NET_HEADER));
    ret += write(sock, &message.role, sizeof(uint8_t));
    ret += write(sock, message.name, (size-1));

    if(ret != gsize) {
        perror("write");
        printf("%d bytes\n", ret);
        fflush(stdout);
    }
}

/*
void send_board_content(int sock, char *buffer) {
    int ret;
    int size;
	int gsize;

    struct NET_MESSAGE_BOARD_CONTENT message;
    message.board_content = (char *)malloc((strlen(buffer)+1));
	if(message.board_content == NULL) {
		perror("malloc");
	}
    strcpy(message.board_content, buffer);

    size = strlen(message.board_content);
	gsize = sizeof(struct NET_HEADER) + strlen(message.board_content);

    prepare_message(&message, NET_TYPE_BOARD_CONTENT, size);
    
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
*/

void send_board_clean(int sock) {
    int ret;

    struct NET_MESSAGE_BOARD_CLEAN message;
    prepare_message(&message, NET_TYPE_BOARD_CLEAN, 0);

    printf("Clean board send. Size: %li\n", sizeof(message));
    fflush(stdout);

    ret = 0;
    ret = write(sock, &message, sizeof(message));
    if(ret != sizeof(message)) {
        perror("write");
        printf("%d bytes\n", ret);
        fflush(stdout);
    }
}

void send_request(int sock, uint8_t write_per) {
    int ret;
    int size;

    struct NET_MESSAGE_REQUEST message;
    message.write_per = write_per;

    size = sizeof(message.write_per);

    prepare_message(&message, NET_TYPE_REQUEST, size);
    
	printf("Request send. Size: %li\n", sizeof(message));

    ret = 0;
    ret = write(sock, &message, sizeof(message));
    if(ret != sizeof(message)) {
        perror("write");
        printf("%d bytes\n", ret);
        fflush(stdout);
    }
}

void send_reply(int sock, uint8_t write_per, uint16_t client_id) {
    int ret;
    int size;

    struct NET_MESSAGE_REPLY message;
    message.write_per = write_per;
	message.client_id = htons(client_id);

    size = sizeof(message.write_per) + sizeof(message.client_id);

    prepare_message(&message, NET_TYPE_REPLY, size);
    
	printf("Reply send. Size: %li\n", sizeof(message));

    ret = 0;
    ret = write(sock, &message, sizeof(message));
    if(ret != sizeof(message)) {
        perror("write");
        printf("%d bytes\n", ret);
        fflush(stdout);
    }
}

void send_shutdown(int sock) {
    int ret;

    struct NET_MESSAGE_SHUTDOWN message;
    prepare_message(&message, NET_TYPE_SHUTDOWN, 0);

    printf("Shutdown send. Size: %li\n", sizeof(message));
    fflush(stdout);

    ret = 0;
    ret = write(sock, &message, sizeof(message));
    if(ret != sizeof(message)) {
        perror("write");
        printf("%d bytes\n", ret);
        fflush(stdout);
    }
}
