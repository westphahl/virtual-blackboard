#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

void* client_handler(void *sfd) {

    // TODO TESTING
    int *fd = (int *) sfd;
    int socket_fd = *fd;
    int ret;
    static char buf[512];

    while (1) {
        ret = read(socket_fd, buf, sizeof(buf));
        if (ret == 0) {
            break;
        }
        if (ret < 0) {
            perror("read");
            break;
        }
        if (write(socket_fd, buf, ret) < ret) {
            perror("write");
            break;
        }
    }
    close(socket_fd);
    // TODO TESTING

    pthread_exit(0);
    return NULL;
}
