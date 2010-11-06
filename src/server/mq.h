#ifndef MQ_H
#define MQ_H

#define MSGSIZE 255
#define MSGTYPE 12345

struct logmessage {
    long type;
    int level;
    time_t time;
    char message[MSGSIZE];
};

int create_mq(key_t key);
int get_mq(key_t key);
void delete_mq(int mq_id);

struct logmessage read_mq(int mq_id);
void write_mq(int mq_id, struct logmessage buffer);

#endif
