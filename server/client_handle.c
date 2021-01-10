#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>

#include "client_handle.h"
#include "logger.h"
#include "packets.h"

#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

extern struct nfs_logger* logger;
extern char* nfs_path;
struct client clients[MAX_CLIENTS_COUNT];

char* sanitize_path(char* path) {
    size_t path_length = strlen(path);
    size_t nfs_path_length = strlen(nfs_path);

    char* combined_path = calloc(1, path_length + nfs_path_length + 2);
    strcpy(combined_path, nfs_path);
    combined_path[nfs_path_length] = '/';
    strncpy(&combined_path[nfs_path_length + 1], path, path_length);

    // Normalizing path would be better... Maybe later
    char* normalized_path = combined_path;

    if(strstr(normalized_path, "/../")) {
        free(normalized_path);
        return NULL;
    }
    // Check whether normalized path starts with path to the shared directory
    if(strncmp(normalized_path, nfs_path, nfs_path_length - 1)) {
        free(normalized_path);
        return NULL;
    }

    return normalized_path;
}

static struct client* get_client_by_socket(int socket_fd) {
    for(int i = 0; i < ARRAY_SIZE(clients); i++)
        if(clients[i].active && clients[i].socket_fd == socket_fd)
            return &clients[i];
    return NULL;
}

int add_new_client(int socket_fd) {
    for(int i = 0; i < ARRAY_SIZE(clients); i++) {
        if(!clients[i].active) {
            clients[i].active = 1;
            clients[i].socket_fd = socket_fd;
            clients[i].opened_file_fd = -1;
            update_timeout(socket_fd);

            return MYNFS_SUCCESS;
        }
    }

    nfs_log_error(logger, "Failed to add new client - too many open sessions");
    return MYNFS_OVERLOAD;
}

static void close_client(struct client* client) {
    close(client->opened_file_fd);
    client->opened_file_fd = -1;
    client->active = 0;
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

    if(packet->cmd != MYNFS_CMD_OPEN && packet->cmd != MYNFS_CMD_UNLINK) {
        if(client->opened_file_fd < 0) {
            nfs_log_error(logger, "Attempted to execute command without openning file: %d", packet->cmd);
            return MYNFS_INVALID_PACKET;
        }

        if(packet->handle != client->opened_file_fd) {
            nfs_log_error(logger, "Invalid handle provided by user (%d)", packet->handle);
            return MYNFS_INVALID_CLIENT;
        }
    }

    nfs_log_debug(logger, "Processing command ID(%d) from packet of data size %d", packet->cmd, packet->data_length);
    switch(packet->cmd) {
        case MYNFS_CMD_OPEN: {
            int lock_file = 0;
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

            char* path_name = strndup((char*)open_data->name, open_data->path_length);
            char* sanitized_path = sanitize_path(path_name);

            if(sanitized_path == NULL) {
                nfs_log_error(logger, "User provided path is invalid - %s", path_name);
                free(path_name);
                return MYNFS_INVALID_PATH;
            }
            free(path_name);

            // Handle lock flag
            if(open_data->oflag & O_MYNFS_LOCK) {
                nfs_log_debug(logger, "File lock has been requested by the client");
                lock_file = 1;
                open_data->oflag &= ~O_MYNFS_LOCK;
            }

            client->opened_file_fd = open(sanitized_path, open_data->oflag, open_data->mode);
            if(client->opened_file_fd < 0) {
                nfs_log_error(logger, "Failed to open file `%s` - errno: %d", sanitized_path, errno);
                free(sanitized_path);
                return (errno > 0) ? -1 * errno : errno;
            }
            free(sanitized_path);

            if(lock_file) {
                int ret = flock(client->opened_file_fd, LOCK_EX | LOCK_NB);
                if(ret < 0) {
                    close(client->opened_file_fd);
                    client->opened_file_fd = -1;
                    if(errno == EWOULDBLOCK) {
                        nfs_log_debug(logger, "Failed to lock file - file is already locked");
                        return MYNFS_ALREADY_LOCKED;
                    } else {
                        nfs_log_error(logger, "Failed to lock file - flock returned %s", strerror(errno));
                        return (errno > 0) ? -1 * errno : errno;
                    }
                }
            }
            return client->opened_file_fd;
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

        case MYNFS_CMD_WRITE: {
            struct mynfs_write_t* write_data = (struct mynfs_write_t*) packet->data;
            if(packet->data_length < sizeof(*write_data) 
                    || packet->data_length < sizeof(*write_data) + write_data->length) {
                nfs_log_error(logger, "Invalid size of packet mynfs_write_t (%d)", packet->data_length);
                return MYNFS_INVALID_PACKET;
            }

            size_t current_pos = 0;
            while(current_pos < write_data->length) {
                int ret = write(client->opened_file_fd, write_data->buffer + current_pos, write_data->length - current_pos);
                if(ret < 0) {
                    nfs_log_error(logger, "Failed to write file - errno: %d", errno);
                    return (errno > 0) ? -1 * errno : errno;
                }

                current_pos += ret;
            }

            return current_pos;
        }
        break;

        case MYNFS_CMD_LSEEK: {
            struct mynfs_lseek_t* lseek_data = (struct mynfs_lseek_t*) packet->data;
            if(packet->data_length < sizeof(*lseek_data)) {
                nfs_log_error(logger, "Invalid size of packet mynfs_lseek_t (%d)", packet->data_length);
                return MYNFS_INVALID_PACKET;
            }

            int ret = lseek(client->opened_file_fd, lseek_data->offset, lseek_data->whence);
            if(ret < 0) {
                nfs_log_error(logger, "Failed to lseek file - errno: %d", errno);
                return (errno > 0) ? -1 * errno : errno;
            }
            return ret;
        }
        break;

        case MYNFS_CMD_FSTAT: {
            if(packet->data_length != 0)
                nfs_log_warn(logger, "Expected packet to be empty for FSTAT command, but size is '%d'", packet->data_length);
            
            *response = calloc(1, sizeof(struct stat));
            if(*response == NULL) {
                nfs_log_error(logger, "Memory allocation failure - %s:%d", __func__, __LINE__);
                return MYNFS_OVERLOAD;
            }

            int ret = fstat(client->opened_file_fd, (struct stat *)*response);
            if(ret < 0) {
                nfs_log_error(logger, "Failed to fstat file - errno: %d", errno);
                free(*response);
                return (errno > 0) ? -1 * errno : errno;
            }

            *response_size = sizeof(struct stat);
            return ret;
        }
        break;

        case MYNFS_CMD_UNLINK: {
            struct mynfs_unlink_t* unlink_data = (struct mynfs_unlink_t*) packet->data;
            if(packet->data_length < sizeof(*unlink_data) 
                    || packet->data_length < sizeof(*unlink_data) + unlink_data->path_length) {
                nfs_log_error(logger, "Invalid size of packet mynfs_unlink_t (%d)", packet->data_length);
                return MYNFS_INVALID_PACKET;
            }

            char* path_name = strndup((char*)unlink_data->name, unlink_data->path_length);
            char* sanitized_path = sanitize_path(path_name);
            if(sanitized_path == NULL) {
                nfs_log_error(logger, "User provided path is invalid - %s", path_name);
                free(path_name);
                return MYNFS_INVALID_PATH;
            }
            free(path_name);


            int ret = unlink(sanitized_path);
            if(ret < 0) {
                nfs_log_error(logger, "Failed to unlink file `%s` - errno: %d", sanitized_path, errno);
                free(sanitized_path);
                return (errno > 0) ? -1 * errno : errno;
            }

            free(sanitized_path);
            return MYNFS_SUCCESS;
        }
        break;

        case MYNFS_CMD_CLOSE: {
            close_client(client);
            return MYNFS_CLOSED;
        }

        default: {
            nfs_log_error(logger, "Unknown command provided by user (%d)", packet->cmd);
            return MYNFS_UNKNOWN_COMMAND;
        }
        break;
    }
    
    // Each case should return sth
    assert(0);
}

int process_client_message(int socket_fd, void* packet, size_t packet_size, void** response, size_t* response_size) {
    int ret = _process_client_message(socket_fd, packet, packet_size, (char**)response, response_size);
    
    if(*response_size == 0) {
        create_simple_response(0, ret, (struct mynfs_datagram_t**)response, response_size);
    } else {
        struct mynfs_datagram_t* full_packet = calloc(1, *response_size + sizeof(*full_packet));
        if(full_packet == NULL) {
            nfs_log_error(logger, "Memory allocation failure - %s:%d", __func__, __LINE__);
            *response = NULL;
            *response_size = 0;
            return MYNFS_OVERLOAD;
        }

        full_packet->return_value = ret;
        full_packet->data_length = *response_size;
        memcpy(full_packet->data, *response, *response_size);

        free(*response);
        *response = full_packet;
        *response_size += sizeof(*full_packet);
    }

    return ret;
}


void update_timeout(int socket_fd) {
    struct client* client = get_client_by_socket(socket_fd);
    if(client == NULL)
        return;
    client->time_left = time(NULL) + CLIENT_TIMEOUT_VALUE;
}


int check_timeouts(void) {
    unsigned long current_time = time(NULL);
    for(int i = 0; i < ARRAY_SIZE(clients); i++)
        if(clients[i].active && clients[i].time_left < current_time) {
            close_client(&clients[i]);
            return clients[i].socket_fd;
        }
    return -1;
}

