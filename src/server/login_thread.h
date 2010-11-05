#ifndef LOGIN_THREAD_H
#define LOGIN_THREAD_H

/*
 * This structure holds all the socket descriptors to 
 * listen on and the number of valid sockets and is
 * passed to the function as argument of phtread_create().
 */
struct logint_data {
    int *fds;
    int fd_count;
};

void* login_thread(void *data);

#endif
