#ifndef HANDLESOCKETS_H
#define HANDLESOCKETS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include "../include/packets.h"

int createSocket(char *serverIp, uint16_t port);
int sendAndGetResponse(int socketFd, mynfs_datagram_t *clientRequest, mynfs_datagram_t **serverResponse);
int closeSocket(int socketFd);

#endif //HANDLESOCKETS_H