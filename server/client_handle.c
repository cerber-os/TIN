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

static void create_simple_response(int cmd, int ret_val, struct mynfs_datagram_t** response, size_t* response_size) {
    *response = calloc(1, sizeof(**response));
    if(*response == NULL) {
        nfs_log_error(logger, "Memory allocation failure - %s:%d", __func__, __LINE__);
        *response_size = 0;
        return;
    }

    (*response)->cmd = cmd;
    (*response)->data_length = 0;
    (*response)->return_value = ret_val;
    *response_size = sizeof(**response);
}


static int _process_client_message(int socket_fd, struct mynfs_datagram_t* packet, size_t packet_size, 
            char** response, size_t* response_size) {
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
            // TODO: Sanitize path_name

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
                return MYNFS_INVALID_PACKET;
            }

            size_t read_length = (read_data->length > MAX_READ_SIZE) ? MAX_READ_SIZE : read_data->length;
            *response = calloc(1, read_length);
            if(*response == NULL) {
                nfs_log_error(logger, "Memory allocation failure - %s:%d", __func__, __LINE__);
                return MYNFS_OVERLOAD;
            }
            
            int ret = read(client->opened_file_fd, *response, read_length);
            if(ret < 0) {
                nfs_log_error(logger, "Failed to read file - errno: %d", errno);
                free(*response);
                *response = 0;
                return (errno > 0) ? -1 * errno : errno;
            }

            *response_size = ret;
            return ret;
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

int process_client_message(int socket_fd, char* packet, size_t packet_size, char** response, size_t* response_size) {
    int ret = _process_client_message(socket_fd, packet, packet_size, response, response_size);
    
    if(*response_size == 0) {
        create_simple_response(0, ret, response, response_size);
    } else {
        struct mynfs_datagram_t* full_packet = calloc(1, *response_size + sizeof(*full_packet));
        if(full_packet == NULL) {
            // TODO: Error handling...
        }

        full_packet->return_value = ret;
        full_packet->data_length = *response_size;
        memcpy(full_packet->data, *response, *response_size);

        free(*response);
        *response = full_packet;
    }

    return ret;
}


void update_timeout(int socket_fd) {
    // TODO: Update timeout
}


int add_new_client(int socket_fd) {
    for(int i = 0; i < ARRAY_SIZE(clients); i++) {
        if(!clients[i].active) {
            clients[i].active = 1;
            clients[i].socket_fd = socket_fd;
            clients[i].opened_file_fd = -1;
            clients[i].time_left = -1;          // TODO

            return MYNFS_SUCCESS;
        }
    }

    nfs_log_error(logger, "Failed to add new client - too many open sessions");
    return MYNFS_OVERLOAD;
}

static void close_client(struct client* client) {
    // TODO:
    nfs_log_error(logger, "Not implemented yet");
    return;
}

void check_timeouts(void) {
    // TODO:
    return;
}

