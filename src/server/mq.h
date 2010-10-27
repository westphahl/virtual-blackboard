#ifndef MQ_H
#define MQ_H

#define MSGSIZE 255
#define MSGTYPE 805

struct logmessage {
    long type;
    time_t time;
    char message[MSGSIZE];
};
typedef struct logmessage LOGMESSAGE;

int create_mq(key_t key);
int get_mq(key_t key);
void delete_mq(int mq_id);

#endif
