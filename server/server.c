#include <security/pam_appl.h>
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
int port_number, colored_logs, client_queue, log_level;
char* hostname, * nfs_path, * log_file_location;

struct config_field fields[] = {
    {.name = "HOSTNAME", .type = CONFIG_TYPE_STRING, .dst = &hostname, .mandatory = 1},
    {.name = "PORT", .type = CONFIG_TYPE_INT, .dst = &port_number, .mandatory = 1},
    {.name = "CLIENT_QUEUE", .type = CONFIG_TYPE_INT, .dst = &client_queue, .mandatory = 1},
    {.name = "NFS_PATH", .type = CONFIG_TYPE_STRING, .dst = &nfs_path, .mandatory = 1},
    {.name = "LOG_FILE", .type = CONFIG_TYPE_STRING, .dst = &log_file_location, .mandatory = 1},
    {.name = "LOG_LEVEL", .type = CONFIG_TYPE_INT, .dst = &log_level},
    {.name = "COLORED_LOGS", .type = CONFIG_TYPE_BOOL, .dst = &colored_logs},
};


/*
 * PAM authentication support
 */
int func_conv(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *appdata_ptr)
{
    struct pam_response *pam_reply;

    pam_reply = calloc(1, sizeof(struct pam_response));
    if(pam_reply == NULL)
        return PAM_BUF_ERR;
    pam_reply->resp = strdup(appdata_ptr);
    pam_reply->resp_retcode = 0;

    *resp = pam_reply;
    return PAM_SUCCESS;
}

int check_credentials(char* username, char* password) {
    int ret = 0;
    const struct pam_conv local_conversation = { func_conv, password };
    
    pam_handle_t *local_auth_handle = NULL;
    int retval = pam_start("system-auth", username, &local_conversation, &local_auth_handle);
    if(retval != PAM_SUCCESS) {
        nfs_log_info(logger, "Failed to start PAM transaction - error: %d", retval);
        goto err;
    }

    retval = pam_authenticate(local_auth_handle, 0);
    if(retval != PAM_SUCCESS) {
        if(retval == PAM_AUTH_ERR || retval == PAM_USER_UNKNOWN || retval == PAM_PERM_DENIED) {
            nfs_log_error(logger, "Failed to authenticate user `%s` - invalid username/password", username);
            goto err_pam_end;
        } else {
            nfs_log_error(logger, "Failed to authenticate user `%s` - PAM error (%d)", username, retval);
            goto err_pam_end;
        }
    }

    // retval == PAM_SUCCESS - username/password are correct
    ret = 1;

err_pam_end:
    retval = pam_end(local_auth_handle, retval);
    if(retval != PAM_SUCCESS)
        nfs_log_error(logger, "Failed to end PAM transaction - PAM error (%d)", retval);
err:
    return ret;
}


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
    int main_sock, sub_socket, max_fd, ready_sockets, rv;
    struct sockaddr_in server;
    fd_set read_fds;
    char **response;
    size_t response_len;
    struct timeval timeout;
    char buffer[MAX_BUF];
    
    main_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (main_sock == -1) 
    {
        nfs_log_error(logger, "Failed to open stream socket: %s", strerror(errno));
        exit(1);
    }
    else
	    max_fd = main_sock + 1;
    
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

    listen(main_sock, client_queue);
    do {
        FD_ZERO(&read_fds);
        FD_SET(sockets_list->fd, &read_fds);
        temp = sockets_list;
        while(temp->next)
        {
            temp = temp->next;
            FD_SET(temp->fd, &read_fds);
        }
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if((ready_sockets = select(max_fd, &read_fds, NULL, NULL, &timeout)) == -1) 
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
                    list_add(sockets_list, sub_socket);
                    nfs_log_info(logger, "New connection accepted");
                }
            }
            if (sub_socket == -1)
                nfs_log_error(logger, "Failed to accept: %s", strerror(errno));
        }
        temp = sockets_list;
        while(temp->next)
        {
            temp = temp->next;
            if (FD_ISSET(temp->fd, &read_fds)) 
            {
                if((rv = read(sub_socket, buffer, MAX_BUF)) == -1)
                {
                    nfs_log_error(logger, "Failed to read: %s", strerror(errno));
                    continue;
                }   
                if (rv == 0) 
                {
                    nfs_log_warn(logger, "Socket was ready to read, but we didn't get any data. Strange!");
                    continue;
                }

                update_timeout(sub_socket);

                response = NULL;
                response_len = 0;
                rv = process_client_message(sub_socket, buffer, rv, (void**) response, &response_len);
                if(rv == MYNFS_CLOSED)
                {
                    free(response);
                    close(sub_socket);
                    list_remove_by_fd(&sockets_list, sub_socket);
                    nfs_log_info(logger, "Connection with remote client closed");
                    continue;
                }
                else
                {
                    if((rv = write(sub_socket, response, response_len)) == -1)
                        nfs_log_error(logger, "Failed to write: %s", strerror(errno));

                    free(response);
                }
            }
        }

        // Monitor for timeouts
        int timedout_client_fd = check_timeouts();
        if(timedout_client_fd >= 0) {
            close(timedout_client_fd);
            list_remove_by_fd(&sockets_list, timedout_client_fd);
            nfs_log_info(logger, "Disconnected client - timeout");
        }
    } while(1);
}
