#ifndef HANDLESOCKETS_H
#define HANDLESOCKETS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include "../include/packets.h"

int sendMessageAndGetResponse(char *serverIp, uint16_t port, mynfs_datagram_t *clientRequest, mynfs_datagram_t **serverResponse);

#endif //HANDLESOCKETS_H