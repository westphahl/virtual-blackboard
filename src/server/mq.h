#ifndef MQ_H
#define MQ_H

struct logmessage {
    int level;
    time_t time;
    char *message;
};
typedef struct logmessage LOGMESSAGE;

int create_mq(key_t key);
int get_mq(key_t key);
void delete_mq(int mq_id);

#endif
