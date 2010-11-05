#ifndef LIVEAGENT_THREAD_H
#define LIVEAGENT_THREAD_H

struct liveagentt_data {
    int socket;
};

void handler(int signum);
void* liveagent_handler(void* data);

#endif
