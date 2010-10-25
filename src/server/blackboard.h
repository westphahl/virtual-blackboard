#ifndef BLACKBOARD_H
#define BLACKBOARD_H

int init_blackboard(key_t key);

int get_blackboard(key_t key);

void delete_blackboard(int shmid);

#endif
