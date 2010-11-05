#ifndef COMMAND_THREAD_H
#define COMMAND_THREAD_H

#include <netinet/in.h>

void prepare_message(void *hdr, int type, int lenght);

void send_login(int sock, char *name, uint8_t role);
void send_board_content(int sock, char *buffer);
void send_board_clean(int sock);
void send_request(int sock, uint8_t write_per);
void send_reply(int sock, uint8_t write_per, uint16_t client_id);
void send_shutdown(int sock);

#endif
