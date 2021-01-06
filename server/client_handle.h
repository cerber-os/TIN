#include <stdlib.h>

#define MAX_CLIENTS_COUNT       (16)
#define MAX_READ_SIZE           (4096)

struct client {
    int active;

    int socket_fd;
    int opened_file_fd;
    int time_left;
};

struct client clients[MAX_CLIENTS_COUNT];

void add_new_client(void);
int process_client_message(int socket_fd, struct mynfs_datagram_t* packet, size_t packet_size, 
            struct mynfs_datagram_t** response, size_t* response_size);
void update_timeout(void);
void close_client(void);

