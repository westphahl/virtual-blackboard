#define _BSD_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/* 
 * Create a new lockfile
 * The function returns the file descriptor of the lockfile
 */
int create_lock() {
    int file;
    char s[32]; // Buffer for pid
    struct flock lock;

    file = open("server.lock", O_WRONLY | O_CREAT, 0644);
    if (file < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* Prepare struct for lock */
    lock.l_type = F_WRLCK; // Write lock
    lock.l_whence = SEEK_SET; // Set to start of file
    lock.l_start = 0; // Starting offset for lock
    lock.l_len = 0; // Number of bytes to lock

    /* Create lock if possible */
    if (fcntl(file, F_SETLK, &lock) < 0) {
        fprintf(stdout, "There is another server instance running!\n");
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    /* Truncate the file to 0 bytes */
    if (ftruncate(file, 0) < 0) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    /* Write process id to file */
    snprintf(s, sizeof(s), "%d\n", (int) getpid());
    if (write(file, s, strlen(s)) < strlen(s)) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Sync changes to disc */
    if (fsync(file) < 0) {
        perror("fsync");
        exit(EXIT_FAILURE);
    }
    
    return file;
}

/*
 * Close and delete the lockfile specified by the given
 * file descriptor.
 */
void delete_lock(int file) {
    /* Delete lockfile */
    close(file);
    if (unlink("server.lock")) {
        perror("unlink");
    }
}
