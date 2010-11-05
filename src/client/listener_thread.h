#ifndef LISTENER_THREAD_H
#define LISTENER_THREAD_H

struct listenert_data {
    int socket;
	struct client_data *cdata;
};

void* listener_handler(void* data);

#endif
