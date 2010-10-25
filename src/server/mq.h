#ifndef MQ_H
#define MQ_H

int create_mq(key_t key);
int get_mq(key_t key);
void delete_mq(int mq_id);

#endif
