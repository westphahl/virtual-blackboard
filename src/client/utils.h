#ifndef UTILS_H_
#define UTILS_H_

void *alloc(size_t size);

void board_lock();
void board_unlock();
void board_destroy();

void cdata_lock();
void cdata_unlock();
void cdata_destroy();

#endif
