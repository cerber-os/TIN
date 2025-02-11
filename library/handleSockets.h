#ifndef HANDLESOCKETS_H
#define HANDLESOCKETS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>


#include "../include/packets.h"

#define MAX_BUF 8192

int createSocket(char *serverIp, uint16_t port);
int sendAndGetResponse(int socketFd, mynfs_message_t *clientRequest, mynfs_message_t **serverResponse);
int closeSocket(int socketFd);

#endif //HANDLESOCKETS_H