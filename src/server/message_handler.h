#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <stdint.h>

int login_handler(int sfd);

int board_handler(int sfd, uint16_t length, int mtype);

int request_handler(int sfd);

//int handle_shutdown();

//int handle_reply();

#endif
