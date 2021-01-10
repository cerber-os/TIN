/*
 * Everything releated to processing user packets in server application
 */
#include <stdlib.h>


// Maximum number of active client sessions
#define MAX_CLIENTS_COUNT       (16)
// Maximum size of one READ operation
#define MAX_READ_SIZE           (4096)
// Timeout after which client gets disconnected (5 min)
#define CLIENT_TIMEOUT_VALUE    (60 * 5)


/*
 * client - structure associating client with its socket, opened file descriptor and
 *      time left till timeout
 */
struct client {
    int active;

    int socket_fd;
    int opened_file_fd;
    unsigned long time_left;
};

extern struct client clients[MAX_CLIENTS_COUNT];

/*
 * process_client_message - parse client frame and prepare the response
 *  socket_fd - file descriptor from which packet was recieved
 *  packet - input data
 *  packet_size - size of input data
 *  response - where to save allocated buffer with output data (remember to call free() on it!)
 *  response_size - size of response buffer
 * @returns: one of the `mynfs_error_code` values
 */
int process_client_message(int socket_fd, void* packet, size_t packet_size, void** response, size_t* response_size);

/*
 * update_timeout - refresh timer of client represented by socket FD
 *  socket_fd - file descriptor of client
 */
void update_timeout(int socket_fd);

/*
 * add_new_client - register a new client
 *  socket_fd - file descriptor of client
 * @returns: MYNFS_SUCCESS on success or MYNFS_OVERLOAD if no space for new client is left
 */
int add_new_client(int socket_fd);

/*
 * close_client - remove a client
 *  client - entry of client
 */
void close_client(struct client* client);

/*
 * get_client_by_socket - get struct client entry
 *  socket_fd - file descriptor of client
 * @returns: struct client entry on success or NULL if no client was found
 */
struct client* get_client_by_socket(int socket_fd);

/*
 * check_timeouts - iterate over every active client and close connection of the first one
 *      that has its timeout expired
 * @returns: socket FD to be closed
 */
int check_timeouts(void);

