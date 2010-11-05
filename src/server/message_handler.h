#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <stdint.h>

int login_handler(int sfd);

int board_handler(int sfd, uint16_t length, int mtype);

int request_handler(int sfd);

int reply_handler(int sfd);

#endif
