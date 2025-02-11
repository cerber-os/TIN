#include "fileOperations.h"
using namespace std;

bool check_if_its_number(string str) {
    for (int i = 0; i < str.length(); i++)
        if (isdigit(str[i]) == false)
            return false;
    return true;
}

uint16_t getPortFromString(string host){
    size_t pos = host.find_last_of(':');
    string port = host.substr(pos + 1, host.length() - 1);
    if(!port.empty() && check_if_its_number(port)){
        return (uint16_t)stoul( port );
    }
    std::cerr << "Invalid port input" << std::endl;
    return 0;
}

string getIpFromString (string host){
    size_t pos = host.find_last_of(':');
    string ip = host.substr(0, pos);
    return ip;
}

int mynfs_open(char *host, char *path, int oflag, int mode, int *socketFd){

    //obsluga komunikatu podrzednego
    int path_length = strlen(path);
    int sub_msg_size = sizeof (mynfs_open_t) + path_length + 1;   // +1, bo znak konca null

    mynfs_open_t *clientSubMsg;
    clientSubMsg = (mynfs_open_t *)malloc (sub_msg_size);
    strcpy((char*)clientSubMsg->name, path);
    clientSubMsg->path_length = path_length;
    clientSubMsg->oflag = oflag;
    clientSubMsg->mode = mode;

    //obsluga komunikatu nadrzednego
    mynfs_message_t *clientMsg;
    clientMsg = (mynfs_message_t *)malloc (sizeof (mynfs_message_t) + sub_msg_size);
    memcpy(clientMsg->data, clientSubMsg, sub_msg_size);
    free(clientSubMsg);

    clientMsg->cmd= MYNFS_CMD_OPEN;
    clientMsg->handle = 0;
    clientMsg->data_length = sub_msg_size;

    mynfs_message_t *serverMsg;

    *socketFd = createSocket((char*)getIpFromString(string(host)).c_str(), getPortFromString(string(host)));
    if(*socketFd == -1){
        free(clientMsg);
        return -1;
    }
    if(sendAndGetResponse(*socketFd, clientMsg, &serverMsg) == -1){
        free(clientMsg);
        return -1;
    }

    free(clientMsg);
    return serverMsg->return_value;
}


int mynfs_read(int socketFd, int fd, void *buf, size_t count)
{
    //obsluga komunikatu podrzednego
    mynfs_read_t clientSubMsg;
    clientSubMsg.length = count;

    //obsluga komunikatu nadrzednego
    mynfs_message_t *clientMsg;
    clientMsg = (mynfs_message_t *)malloc (sizeof (mynfs_message_t) + sizeof(mynfs_read_t));
    memcpy(clientMsg->data, &clientSubMsg, sizeof(mynfs_read_t));

    clientMsg->cmd= MYNFS_CMD_READ;
    clientMsg->handle = fd;
    clientMsg->data_length = sizeof(mynfs_read_t);

    mynfs_message_t *serverMsg;

    if(sendAndGetResponse(socketFd, clientMsg, &serverMsg) == -1){
        free(clientMsg);
        return -1;
    }

    memcpy(buf, serverMsg->data, serverMsg->data_length);

    free(clientMsg);
    return serverMsg->return_value;
}


ssize_t mynfs_write(int socketFd, int fd, void *buf, size_t count)
{  
    char *bufor = (char *)buf;

    int buf_length = count;
    int sub_msg_size = sizeof(mynfs_write_t) + buf_length + 1;
    //obsluga komunikatu podrzednego
   
    mynfs_write_t *clientSubMsg =  (mynfs_write_t *)malloc (sub_msg_size);
    clientSubMsg->length = count;
    strcpy((char*)clientSubMsg->buffer, bufor);
    clientSubMsg->length = buf_length;
 
    //obsluga komunikatu nadrzednego
    mynfs_message_t *clientMsg;
    clientMsg = (mynfs_message_t *)malloc (sizeof (mynfs_message_t) + sub_msg_size);
    memcpy(clientMsg->data, clientSubMsg, sub_msg_size);
    free(clientSubMsg);

    clientMsg->cmd= MYNFS_CMD_WRITE;
    clientMsg->handle = fd;
    clientMsg->data_length = sub_msg_size;
    mynfs_message_t *serverMsg;
    if(sendAndGetResponse(socketFd, clientMsg, &serverMsg) == -1){
        free(clientMsg);
        return -1;
    }
 
    free(clientMsg);
    return serverMsg->return_value;
}


off_t mynfs_lseek(int socketFd, int fd, off_t offset, int whence){
    //obsluga komunikatu podrzednego
    mynfs_lseek_t clientSubMsg;
    clientSubMsg.offset = offset;
    clientSubMsg.whence = whence;

    //obsluga komunikatu nadrzednego
    mynfs_message_t *clientMsg;
    clientMsg = (mynfs_message_t *)malloc (sizeof (mynfs_message_t) + sizeof(mynfs_lseek_t));
    memcpy(clientMsg->data, &clientSubMsg, sizeof(mynfs_lseek_t));

    clientMsg->cmd= MYNFS_CMD_LSEEK;
    clientMsg->handle = fd;
    clientMsg->data_length = sizeof(mynfs_lseek_t);

    mynfs_message_t *serverMsg;
    if(sendAndGetResponse(socketFd, clientMsg, &serverMsg) == -1){
        free(clientMsg);
        return -1;
    }

    free(clientMsg);
    return serverMsg->return_value;
}

int mynfs_close(int socketFd, int fd)
{
    //brak komunikatu podrzednego

    //obsluga komunikatu nadrzednego
    mynfs_message_t *clientMsg;
    clientMsg = (mynfs_message_t *)malloc (sizeof (mynfs_message_t));

    clientMsg->cmd= MYNFS_CMD_CLOSE;
    clientMsg->handle = fd;
    clientMsg->data_length = 0;

    mynfs_message_t *serverMsg;
    if(sendAndGetResponse(socketFd, clientMsg, &serverMsg) == -1){
        free(clientMsg);
        return -1;
    }

    free(clientMsg);
    return serverMsg->return_value;
}


int mynfs_unlink(char *host, char *pathname)
{
    //obsluga komunikatu podrzednego
    int path_length = strlen(pathname);
    int sub_msg_size = sizeof (mynfs_unlink_t) + path_length + 1;   // +1, bo znak konca null

    mynfs_unlink_t *clientSubMsg;
    clientSubMsg = (mynfs_unlink_t *)malloc (sub_msg_size);
    strcpy((char*)clientSubMsg->name, pathname);
    clientSubMsg->path_length = path_length;

    //obsluga komunikatu nadrzednego
    mynfs_message_t *clientMsg;
    clientMsg = (mynfs_message_t *)malloc (sizeof (mynfs_message_t) + sub_msg_size);
    memcpy(clientMsg->data, clientSubMsg, sub_msg_size);
    free(clientSubMsg);

    clientMsg->cmd= MYNFS_CMD_UNLINK;
    clientMsg->handle = 0;
    clientMsg->data_length = sub_msg_size;

    mynfs_message_t *serverMsg;
    int socketFd = createSocket((char*)getIpFromString(string(host)).c_str(), getPortFromString(string(host)));
    if(socketFd == -1){
        free(clientMsg);
        return -1;
    }
    if(sendAndGetResponse(socketFd, clientMsg, &serverMsg) == -1){
        free(clientMsg);
        return -1;
    }
    if(closeSocket(socketFd) == -1){
        free(clientMsg);
        return -1;
    }

    free(clientMsg);
    return serverMsg->return_value;
}


int mynfs_fstat(int socketFd, int fd, struct stat *buf)
{
    //brak komunikatu podrzednego
    
    //obsluga komunikatu nadrzednego
    mynfs_message_t *clientMsg;
    clientMsg = (mynfs_message_t *)malloc (sizeof (mynfs_message_t));

    clientMsg->cmd= MYNFS_CMD_FSTAT;
    clientMsg->handle = fd;
    clientMsg->data_length = 0;

    mynfs_message_t *serverMsg;
    if(sendAndGetResponse(socketFd, clientMsg, &serverMsg) == -1){
        free(clientMsg);
        return -1;
    }

    stat *stat_table = (stat *) serverMsg->data;
    memcpy(buf, stat_table, serverMsg->data_length);

    free(clientMsg);
    return serverMsg->return_value;
}