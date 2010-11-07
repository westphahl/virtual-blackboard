#ifndef SHARED_H_
#define SHARED_H_

struct client_data {
	uint16_t cid;
	uint8_t role;
	uint8_t write;
	uint8_t dozenten;
	uint8_t tutoren;
	uint16_t studenten;
	char *name;
};

struct commandt_data {
    int socket;
	struct client_data *cdata;
};

struct listenert_data {
    int socket;
	struct client_data *cdata;
};

struct liveagentt_data {
    int socket;
};

static int pipefd[2];

#endif
