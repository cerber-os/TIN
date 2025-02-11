#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <time.h>
#include <arpa/inet.h>
#include <signal.h>

#include "logger.h"
#include "config_parser.h"
#include "client_handle.h"
#include "packets.h"
#include "list.h"

#define MAX_BUF 8192
#define max(a,b) (((a)>(b)) ? (a):(b))

struct nfs_logger* logger;

/*
 * Config file format specifiers
 */
int port_number, colored_logs, client_queue, log_level, select_timeout, rw_timeout;
char* hostname, * nfs_path, * log_file_location;

struct config_field fields[] = {
    {.name = "HOSTNAME", .type = CONFIG_TYPE_STRING, .dst = &hostname, .mandatory = 1},
    {.name = "PORT", .type = CONFIG_TYPE_INT, .dst = &port_number, .mandatory = 1},
    {.name = "CLIENT_QUEUE", .type = CONFIG_TYPE_INT, .dst = &client_queue, .mandatory = 1},
    {.name = "TIMEOUT_SELECT", .type = CONFIG_TYPE_INT, .dst = &select_timeout, .mandatory = 1},
    {.name = "TIMEOUT_RW", .type = CONFIG_TYPE_INT, .dst = &rw_timeout, .mandatory = 1},
    {.name = "NFS_PATH", .type = CONFIG_TYPE_STRING, .dst = &nfs_path, .mandatory = 1},
    {.name = "LOG_FILE", .type = CONFIG_TYPE_STRING, .dst = &log_file_location, .mandatory = 1},
    {.name = "LOG_LEVEL", .type = CONFIG_TYPE_INT, .dst = &log_level},
    {.name = "COLORED_LOGS", .type = CONFIG_TYPE_BOOL, .dst = &colored_logs},
};


/*
 * Main network filesystem (NFS) server logic
 */
int main(int argc, char** argv) {
    nfs_log_open(&logger, NULL, LOG_LEVEL_INFO, 1);
    nfs_log_info(logger, "myNFS server (V0.1)");

    char* config_file = "./server/example.cfg";
    if(argc < 2) {
        nfs_log_warn(logger, "No location of configuration file provided in argv[1] ...");
        nfs_log_info(logger, "... will use `./server/example.cfg` instead");
    } else {
        config_file = argv[1];
        nfs_log_info(logger, "Reading config file from `%s`", config_file);
    }

    int err = parse_config(fields, sizeof(fields) / sizeof(fields[0]), config_file);
    if(err) {
        nfs_log_error(logger, "Parsing log file failed");
        return 1;
    }

    // Reopen logger after configuration has been read from config file
    nfs_log_close(logger);
    nfs_log_open(&logger, log_file_location, log_level, colored_logs);

    // create listening socket
    int main_sock, sub_socket, max_fd, ready_sockets, rv, sock_opt = 1;
    int disconnect_client;
    struct sockaddr_in server;
    fd_set read_fds;
    char *response;
    size_t response_len, rw_len, header_len, data_len;
    struct timeval timeout_select, timeout_rw;
    char buffer[MAX_BUF];

    timeout_select.tv_sec = select_timeout;
    timeout_select.tv_usec = 0;
    timeout_rw.tv_sec = rw_timeout;
    timeout_rw.tv_usec = 0;
    
    main_sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (main_sock == -1) 
    {
        nfs_log_error(logger, "Failed to open stream socket: %s", strerror(errno));
        exit(1);
    }
    else
	    max_fd = main_sock + 1;
    
    if(setsockopt(main_sock, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)))
    {
        nfs_log_error(logger, "Failed to set socket options: %s", strerror(errno));
        exit(1);
    }
    
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(hostname);
    server.sin_port = htons(port_number);
    if (bind(main_sock, (struct sockaddr *) &server, sizeof server) == -1) 
    {
        nfs_log_error(logger, "Failed to bind stream socket: %s", strerror(errno));
        exit(1);
    }

    nfs_log_info(logger, "Listening on port %d...", port_number);

    list_node *temp, *sockets_list = list_create(main_sock);

    signal(SIGPIPE, SIG_IGN);

    listen(main_sock, client_queue);
    do {
        // Monitor for client timeouts
        int timedout_client_fd;
        while((timedout_client_fd = check_timeouts()) >= 0) {
            close(timedout_client_fd);
            list_remove_by_fd(&sockets_list, timedout_client_fd);
            nfs_log_info(logger, "Disconnected client - timeout");
        }

        timeout_select.tv_sec = select_timeout;
        timeout_select.tv_usec = 0;
        timeout_rw.tv_sec = rw_timeout;
        timeout_rw.tv_usec = 0;
        
        FD_ZERO(&read_fds);
        temp = sockets_list;
        while(temp)
        {
            FD_SET(temp->fd, &read_fds);
            temp = temp->next;
        }
        if((ready_sockets = select(max_fd, &read_fds, NULL, NULL, &timeout_select)) == -1) 
        {
            nfs_log_error(logger, "Failed to select: %s", strerror(errno));
            continue;
        }
        if (ready_sockets == 0)
        {
            nfs_log_debug(logger, "We didn't receive any data. Restarting select...");
            continue;
        }
        if (FD_ISSET(main_sock, &read_fds)) 
        {
            while((sub_socket = accept(main_sock, NULL, NULL)) != -1)
            {
                max_fd = max(max_fd, sub_socket + 1);
                if(add_new_client(sub_socket) ==  MYNFS_OVERLOAD)
                {
                    nfs_log_warn(logger, "Too many clients connected. Refused new connection.");
                    close(sub_socket);
                    break;
                }
                else
                {
                    if(setsockopt(sub_socket, SOL_SOCKET, SO_RCVTIMEO | SO_SNDTIMEO, (const char*)&timeout_rw, sizeof(timeout_rw))
                    || setsockopt(sub_socket, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt)))
                    {
                        nfs_log_error(logger, "Failed to set socket options: %s", strerror(errno));
                        close(sub_socket);
                        continue;
                    }
                    list_add(sockets_list, sub_socket);
                    nfs_log_info(logger, "New connection accepted");
                }
            }
            if (sub_socket == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                nfs_log_debug(logger, "We accepted all new clients");
            else if(sub_socket == -1 && !(errno == EAGAIN || errno == EWOULDBLOCK))
                nfs_log_warn(logger, "Failed to accept: %s", strerror(errno));
        }
        temp = sockets_list->next;
        while(temp)
        {
            if (FD_ISSET(temp->fd, &read_fds)) 
            {
                disconnect_client = 0;
                bzero(buffer, sizeof(buffer));
                rw_len = 0;
                header_len = sizeof(struct mynfs_message_t);
                while(rw_len < header_len)
                {
                    rv = read(temp->fd, buffer + rw_len, header_len - rw_len);
                    if(rv == -1)
                    {
                        nfs_log_error(logger, "Failed to read: %s", strerror(errno));
                        nfs_log_debug(logger, "We tried to read %zu bytes", header_len - rw_len);
                        continue;
                    }
                    else if(rv == 0)
                    {
                        nfs_log_warn(logger, "Read didn't get any data. Client probably disconnected");
                        close_client(get_client_by_socket(temp->fd));
                        close(temp->fd);
                        temp = list_remove_by_fd(&sockets_list, temp->fd);
                        goto disconnected_client;
                    }
                    else
                    {
                        rw_len += rv; 
                        nfs_log_debug(logger, "Read %d bytes. Still waiting for %zu bytes", rw_len, header_len - rw_len);
                    }     
                }

                rw_len = 0;
                data_len = ((struct mynfs_message_t*) buffer)->data_length;
                while(rw_len < data_len)
                {
                    rv = read(temp->fd, buffer + rw_len + header_len, data_len - rw_len);
                    if(rv == -1)
                    {
                        nfs_log_error(logger, "Failed to read: %s", strerror(errno));
                        nfs_log_debug(logger, "We tried to read %zu bytes", data_len - rw_len);
                        continue;
                    }
                    else if(rv == 0)
                    {
                        nfs_log_warn(logger, "Read didn't get any data. Client probably disconnected");
                        close_client(get_client_by_socket(temp->fd));
                        close(temp->fd);
                        temp = list_remove_by_fd(&sockets_list, temp->fd);
                        goto disconnected_client;
                    }
                    else
                    {
                        rw_len += rv; 
                        nfs_log_debug(logger, "Read %d bytes. Still waiting for %zu bytes", rw_len, data_len - rw_len);
                    }              
                }

                update_timeout(temp->fd);

                response = NULL;
                response_len = 0;
                rv = process_client_message(temp->fd, buffer, header_len + data_len, (void**) &response, &response_len);
                if(rv == MYNFS_CLOSED)
                {
                    disconnect_client = 1;
                    nfs_log_debug(logger, "Client will be disconnected");
                }
                
                if(response != NULL)
                {
                    rw_len = 0;
                    while(rw_len < response_len)
                    {
                        rv = write(temp->fd, response + rw_len, response_len - rw_len);
                        if(rv == -1)
                        {
                            nfs_log_error(logger, "Failed to write: %s", strerror(errno));
                            if(errno == EPIPE)
                            {
                                nfs_log_warn(logger, "Broken pipe. Client probably disconnected");
                                close_client(get_client_by_socket(temp->fd));
                                close(temp->fd);
                                temp = list_remove_by_fd(&sockets_list, temp->fd);
                                break;
                            }
                            nfs_log_debug(logger, "We tried to send %zu bytes", response_len - rw_len);
                            continue;
                        }
                        else if(rv == 0)
                        {
                            nfs_log_warn(logger, "We tried to send some data, but we failed. Strange!");
                            continue;
                        }
                        else
                        {
                            rw_len += rv; 
                            nfs_log_debug(logger, "Send %d bytes. Still waiting to send %zu bytes", rv, response_len - rw_len);
                        }              
                    }
                    free(response);
                }
                else
                    nfs_log_error(logger, "Failed to prepare response - response == NULL");

                if(disconnect_client) {
                    // close_client has been called by process_client_message
                    close(temp->fd);
                    temp = list_remove_by_fd(&sockets_list, temp->fd);
                    nfs_log_info(logger, "Connection with remote client closed");
                }
            }
            disconnected_client: temp = temp->next;
        }
    } while(1);
}
