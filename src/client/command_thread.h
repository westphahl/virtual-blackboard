#ifndef COMMAND_THREAD_H_
#define COMMAND_THREAD_H_

#include <netinet/in.h>
#include "../net_message.h"

void send_login(int sock, uint8_t role, char *name);
void send_board_clean(int sock);
void send_request(int sock, uint8_t write_per);
void send_reply(int sock, uint8_t write_per, uint16_t client_id);
void send_shutdown(int sock);

void trigger_command(int type, struct net_reply *reply);
void *command_handler(void *data);

#endif
