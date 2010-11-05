#ifndef BLACKBOARD_H
#define BLACKBOARD_H

int init_blackboard(key_t key);
int get_blackboard(key_t key);
char* blackboard_attach(int shmid);
void blackboard_detach(char *bboard);
void delete_blackboard(int shmid);

#endif
