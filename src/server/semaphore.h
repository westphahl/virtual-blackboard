#ifndef SEMAPHORE_H
#define SEMAPHORE_H

int init_sem(key_t key);
int get_sem(key_t key);
void delete_sem(key_t key);

void lock_sem(int semid);
void unlock_sem(int semid);
void wait_sem(int semid);

#endif
