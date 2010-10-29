#ifndef UTILS_H
#define UTILS_H

int create_semaphore(int key);
void delete_semaphore(int id);
void semaphore_up(int id, struct sembuf buffer);
void semaphore_down(int id, struct sembuf buffer);

#endif
