#ifndef HANDLESOCKETS_H
#define HANDLESOCKETS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#include "msgStructs.h"

int sendMessageAndGetResponse(char *serverIp, uint16_t port, mynfs_msg_t *clientRequest, mynfs_msg_t **serverResponse);

#endif //HANDLESOCKETS_H