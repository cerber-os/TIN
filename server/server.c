#include <security/pam_appl.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/select.h>
#include <time.h>

#include "logger.h"
#include "config_parser.h"
#include "client_handle.h"
#include "packets.h"

#define MAX_FDS 128
#define MAX_BUF 8192
#define max(a,b) (((a)>(b)) ? (a):(b))

struct nfs_logger* logger;

/*
 * Config file format specifiers
 */
int port_number, colored_logs;
char* hostname, * nfs_path;

struct config_field fields[] = {
    {.name = "PORT", .type = CONFIG_TYPE_INT, .dst = &port_number, .mandatory = 1},
    {.name = "HOSTNAME", .type = CONFIG_TYPE_STRING, .dst = &hostname},
    {.name = "NFS_PATH", .type = CONFIG_TYPE_STRING, .dst = &nfs_path},
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
int main() {
    nfs_log_open(&logger, "/tmp/test.txt", LOG_LEVEL_INFO, 1);
    nfs_log_info(logger, "Server version V0.1 starting");

    parse_config(fields, 4, "server/example.cfg");
    nfs_log_info(logger, "Selected port: %d", port_number);

    // create listening socket
    int sock, length;
    struct sockaddr_in server;
    fd_set ready;
    struct timeval timeout;
    int msgsock=-1, nfds, nactive;
    int sockets_table[MAX_FDS];
    char buf[MAX_BUF];

    int rval=0, i;
	for (i=0; i<MAX_FDS; i++) 
        sockets_table[i]=0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("opening stream socket");
        exit(1);
    }
	nfds = sock+1;
    
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(hostname);
    server.sin_port = htons(port_number);
    if (bind(sock, (struct sockaddr *) &server, sizeof server) == -1) 
    {
        perror("binding stream socket");
        exit(1);
    }
    /* wydrukuj na konsoli przydzielony port */
    length = sizeof( server);
    if (getsockname(sock,(struct sockaddr *) &server,&length) == -1) {
        perror("getting socket name");
        exit(1);
    }
    printf("Socket port #%d\n", ntohs(server.sin_port));

    listen(sock, 5);
    do {
        FD_ZERO(&ready);
        FD_SET(sock, &ready);
        for (i=0; i<MAX_FDS; i++) /* dodaj aktywne do zbioru */
            if (sockets_table[i] > 0 )
                FD_SET(sockets_table[i], &ready);
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        if((nactive=select(nfds, &ready, NULL, NULL, &timeout)) == -1) 
        {
             perror("select");
             continue;
        }
 
        if (FD_ISSET(sock, &ready)) 
        {
            msgsock = accept(sock, (struct sockaddr *)0, (int *)0);
            if (msgsock == -1)
                perror("accept");
            nfds=max(nfds, msgsock+1);
            if(nfds > MAX_FDS)
            {
                perror("accept");
                close(msgsock);
            }
            else
            {
                sockets_table[msgsock]=msgsock;
                if(add_new_client(msgsock) ==  MYNFS_OVERLOAD)
                {
                    perror("add_new_client");
                    close(msgsock);
                }
                else
                    printf("accepted...\n");
            }
        } 
        for (i=0; i<MAX_FDS; i++)
            if ((msgsock=sockets_table[i])>0 && FD_ISSET(sockets_table[i], &ready)) 
            {                
                if((rval = read(msgsock, buf, MAX_BUF)) == -1)
                    perror("reading stream message");
                if (rval == 0) 
                {
                    printf("Ending connection\n");
                    close(msgsock);
                    sockets_table[msgsock] = -1;
                    continue;
                }
                char **response = NULL;
                size_t response_len = 0;
                int rv = process_client_message(msgsock, buf, rval, response, &response_len);
                if(rv == MYNFS_CLOSED)
                {
                    close(msgsock);
                    continue;
                }
                else
                {
                    int send_val = 0;
                    if((send_val = write(msgsock, response, response_len)) == -1)
                        perror("writing stream message");
                }
                // int isOk = check_credentials("username", "password");
        }
		if (nactive==0)  
            printf("Nobody is ready. Restarting select...\n");
    } while(1);
}
