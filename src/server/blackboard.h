#ifndef BLACKBOARD_H
#define BLACKBOARD_H

int init_bb_sem(key_t key);
int get_bb_sem(key_t key);
void delete_bb_sem(key_t key);
void lock_bb_sem(int semid);
void unlock_bb_sem(int semid);

int init_blackboard(key_t key);
int get_blackboard(key_t key);
char* blackboard_attach(int shmid);
void blackboard_detach(char *bboard);
void delete_blackboard(int shmid);

#endif
