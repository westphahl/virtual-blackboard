#ifndef CLIENT_H_
#define CLIENT_H_

#include <netinet/in.h>

struct client_data {
	uint16_t cid;
	uint8_t role;
	uint8_t write;
	uint8_t dozenten;
	uint8_t tutoren;
	uint16_t studenten;
	char *name;
};

int updateGUIstate();
void updateBoard(char *text);
char *get_blackboard();

#endif /*CLIENT_H_*/
