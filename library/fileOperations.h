#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <string>
#include <arpa/inet.h>

#include "../include/packets.h"
#include "handleSockets.h"

//Argumenty sa takie same jak w zwyklych linuxowych wersjach tych funkcji + char *host, int port

int mynfs_open(char *host, int port, char *pathname, int flags, mode_t mode);
int mynfs_read(char *host, int port, int fd, void *buf, size_t count);
ssize_t mynfs_write(char *host, int port, int fd, void *buf, size_t count);
off_t mynfs_lseek(char *host, int port, int fd, off_t offset, int whence);
int mynfs_close(char *host, int port, int fd);
int mynfs_unlink(char *host, int port, char *pathname);
int mynfs_fstat(char *host, int port, int fd, struct stat *buf);


#endif //FILEOPERATIONS_H