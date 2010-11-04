#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <stdint.h>

int login_handler(int sfd);

int board_handler(int sfd, uint16_t length, int mtype);

//int handle_board();

//int handle_clear();

//int handle_shutdown();

//int handle_request();

//int handle_reply();

#endif
