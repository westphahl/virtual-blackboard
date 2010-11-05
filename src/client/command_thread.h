#ifndef COMMAND_THREAD_H
#define COMMAND_THREAD_H

#include <netinet/in.h>
#include "../net_message.h"

struct commandt_data {
    int socket;
	struct client_data *cdata;
};

void send_login(int sock, uint8_t role, char *name);
void send_board_clean(int sock);
void send_request(int sock, uint8_t write_per);
void send_reply(int sock, uint8_t write_per, uint16_t client_id);
void send_shutdown(int sock);

void trigger_command(int type, struct net_reply *reply);
void *command_handler(void *data);

#endif
