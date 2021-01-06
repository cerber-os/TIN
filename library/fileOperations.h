#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <string>
#include <arpa/inet.h>

#include "msgStructs.h"
#include "handleSockets.h"

//Argumenty sa takie same jak w zwyklych linuxowych wersjach tych funkcji

int mynfs_open(const char *pathname, const char *flags, mode_t mode);

int mynfs_read(int fd, void *buf, size_t count);
ssize_t mynfs_write(int fd, const void *buf, size_t count);
off_t mynfs_lseek(int fd, off_t offset, int whence);
int mynfs_close(int fd);
int mynfs_unlink(const char *pathname);
int mynfs_fstat(int fd, struct stat *buf);


#endif //FILEOPERATIONS_H