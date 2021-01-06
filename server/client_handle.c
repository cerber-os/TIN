#include <errno.h>
#include <string.h>

#include "client_handle.h"
#include "logger.h"
#include "packets.h"

#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

extern struct nfs_logger* logger;

static struct client* get_client_by_socket(int socket_fd) {
    for(int i = 0; i < ARRAY_SIZE(clients); i++)
        if(clients[i].active && clients[i].socket_fd == socket_fd)
            return &clients[i];
    return NULL;
}


void add_new_client(void);

int process_client_message(int socket_fd, struct mynfs_datagram_t* packet, size_t packet_size, 
            struct mynfs_datagram_t** response, size_t* response_size) {
    *response = NULL;
    *response_size = 0;
    
    struct client* client = get_client_by_socket(socket_fd);
    if(client == NULL) {
        nfs_log_error(logger, "Unknown client with socket_fd (%d)", socket_fd);
        return MYNFS_INVALID_CLIENT;
    }

    if(packet_size < sizeof(*packet) || packet_size < sizeof(*packet) + packet->data_length) {
        nfs_log_error(logger, "Invalid packet size - Got: %d", packet_size);
        return MYNFS_INVALID_PACKET;
    }

    if(packet->cmd != MYNFS_CMD_OPEN && packet->handle != client->opened_file_fd) {
        nfs_log_error(logger, "Invalid handle provided by user (%d)", packet->handle);
        return MYNFS_INVALID_CLIENT;
    }

    switch(packet->cmd) {
        case MYNFS_CMD_OPEN: {
            struct mynfs_open_t* open_data = (struct mynfs_open_t*) packet->data;
            if(packet->data_length < sizeof(*open_data) 
                    || packet->data_length < sizeof(*open_data) + open_data->path_length) {
                nfs_log_error(logger, "Invalid size of packet mynfs_open_t (%d)", packet->data_length);
                return MYNFS_INVALID_PACKET;
            }
            if(client->opened_file_fd >= 0) {
                nfs_log_error(logger, "Client has already opened file");
                return MYNFS_ALREADY_OPENED;
            }

            char* path_name = strndup(open_data->name, open_data->path_length);
            // TODO: Sanitaze path_name

            client->opened_file_fd = open(path_name, open_data->oflag, open_data->mode);
            if(client->opened_file_fd < 0) {
                nfs_log_error(logger, "Failed to open file `%s` - errno: %d", path_name, errno);
                free(path_name);
                return (errno > 0) ? -1 * errno : errno;
            }

            // TODO: chown

            free(path_name);
        }
        break;
    
        case MYNFS_CMD_READ: {
            struct mynfs_read_t* read_data = (struct mynfs_read_t*) packet->data;
            if(packet->data_length < sizeof(*read_data)) {
                nfs_log_error(logger, "Invalid size of packet mynfs_read_t (%d)", packet->data_length);
            }

            size_t read_length = (read_data->length > MAX_READ_SIZE) ? MAX_READ_SIZE : read_data->length;

        }
        break;


        // TODO: the rest

        default: {
            nfs_log_error(logger, "Unknown command provided by user (%d)", packet->cmd);
            return MYNFS_UNKNOWN_COMMAND;
        }
        break;
    }
    
    return MYNFS_SUCCESS;
}

void update_timeout(void);
void close_client(void);

