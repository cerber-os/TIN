#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <string>
#include <arpa/inet.h>
#include <iostream>

#include "../include/packets.h"
#include "handleSockets.h"

#ifdef __cplusplus
extern "C" {
#endif

    int mynfs_open(char *host, char *path, int oflag, int mode, int *socketFd);
    int mynfs_read(int socketFd, int fd, void *buf, size_t count);
    ssize_t mynfs_write(int socketFd, int fd, void *buf, size_t count);
    off_t mynfs_lseek(int socketFd, int fd, off_t offset, int whence);
    int mynfs_close(int socketFd, int fd);
    int mynfs_unlink(char *host, char *pathname);
    int mynfs_fstat(int socketFd, int fd, struct stat *buf);

#ifdef __cplusplus   
}
#endif

#endif //FILEOPERATIONS_H