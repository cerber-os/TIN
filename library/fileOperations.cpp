#include "fileOperations.h"


//"należy zaimplementować co najmniej następujące tryby otwarcia pliku: O_RDONLY, O_WRONLY, O_RDWR, O_CREAT"
int mynfs_open(char *host, int port, const char *pathname, int flags, mode_t mode){

    //ogarnianie komunikatu podrzednego
    int path_length = strlen(pathname);
    int sub_msg_size = sizeof (mynfs_open_t) + path_length + 1;   // +1, bo znak konca null

    mynfs_open_t *clientSubMsg;
    clientSubMsg = (mynfs_open_t *)malloc (sub_msg_size);
    strcpy(clientSubMsg->name, pathname);
    clientSubMsg->path_length = path_length;
    clientSubMsg->oflag = flags;
    clientSubMsg->mode = mode;

    //ogarnianie komunikatu nadrzednego
    mynfs_msg_t *clientMsg;
    clientMsg = (mynfs_msg_t *)malloc (sizeof (mynfs_msg_t) + sub_msg_size);
    memcpy(clientMsg->data, clientSubMsg, sub_msg_size);

    clientMsg->cmd= 0;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg->handle = 0;
    clientMsg->data_length = sub_msg_size;

    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, port, clientMsg, &serverMsg);

    // TODO: sprawdzic czy nie trzeba uzyc tu "ntohl"
    // return serverMsg->return_value;
    return 0;
}


int mynfs_read(char *host, int port, int fd, void *buf, size_t count)
{
    //ogarnianie komunikatu podrzednego
    mynfs_read_t clientSubMsg;
    clientSubMsg.length = count;

    //ogarnianie komunikatu nadrzednego
    mynfs_msg_t *clientMsg;
    clientMsg = (mynfs_msg_t *)malloc (sizeof (mynfs_msg_t) + sizeof(mynfs_read_t));
    memcpy(clientMsg->data, &clientSubMsg, sizeof(mynfs_read_t));

    clientMsg->cmd= 1;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg->handle = fd;
    clientMsg->data_length = sizeof(mynfs_read_t);

    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, port, clientMsg, &serverMsg);

    // int len = ntohl(serverMsg->data_length);
    // memcpy(buf, serverMsg->data, len);
    // return len;

    return 0;
}


ssize_t mynfs_write(char *host, int port, int fd, const void *buf, size_t count)
{  
    char *bufor = (char *)buf;

    int buf_length = strlen(bufor);
    int sub_msg_size = sizeof(mynfs_write_t) + buf_length + 1;
    //ogarnianie komunikatu podrzednego
   
    mynfs_write_t *clientSubMsg =  (mynfs_write_t *)malloc (sub_msg_size);
    clientSubMsg->length = count;
    strcpy(clientSubMsg->buffer, bufor);
    clientSubMsg->length = buf_length;
 
    //ogarnianie komunikatu nadrzednego
    mynfs_msg_t *clientMsg;
    clientMsg = (mynfs_msg_t *)malloc (sizeof (mynfs_msg_t) + sub_msg_size);
    memcpy(clientMsg->data, clientSubMsg, sub_msg_size);
 
    clientMsg->cmd= 2;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg->handle = fd;
    clientMsg->data_length = sub_msg_size;
    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, port, clientMsg, &serverMsg);
 
    // return serverMsg->return_value;
    return 0;
}


off_t mynfs_lseek(char *host, int port, int fd, off_t offset, int whence){
    //ogarnianie komunikatu podrzednego
    mynfs_lseek_t clientSubMsg;
    clientSubMsg.offset = offset;
    clientSubMsg.whence = whence;

    //ogarnianie komunikatu nadrzednego
    mynfs_msg_t *clientMsg;
    clientMsg = (mynfs_msg_t *)malloc (sizeof (mynfs_msg_t) + sizeof(mynfs_lseek_t));
    memcpy(clientMsg->data, &clientSubMsg, sizeof(mynfs_lseek_t));

    clientMsg->cmd= 3;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg->handle = fd;
    clientMsg->data_length = sizeof(mynfs_lseek_t);

    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, port, clientMsg, &serverMsg);

    // return value powinno byc offsetem
    // return serverMsg->return_value;
    return 0;
}

int mynfs_close(char *host, int port, int fd)
{
    //brak komunikatu podrzednego

    //ogarnianie komunikatu nadrzednego
    mynfs_msg_t *clientMsg;
    clientMsg = (mynfs_msg_t *)malloc (sizeof (mynfs_msg_t));

    clientMsg->cmd= 4;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg->handle = fd;
    clientMsg->data_length = 0;

    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, port, clientMsg, &serverMsg);

    //TODO: obsluga kodow o bledzie z serwera
    return 0;   //0 = success
}


int mynfs_unlink(char *host, int port, const char *pathname)
{
    //ogarnianie komunikatu podrzednego
    int path_length = strlen(pathname);
    int sub_msg_size = sizeof (mynfs_unlink_t) + path_length + 1;   // +1, bo znak konca null

    mynfs_unlink_t *clientSubMsg;
    clientSubMsg = (mynfs_unlink_t *)malloc (sub_msg_size);
    strcpy(clientSubMsg->name, pathname);
    clientSubMsg->path_length = path_length;

    //ogarnianie komunikatu nadrzednego
    mynfs_msg_t *clientMsg;
    clientMsg = (mynfs_msg_t *)malloc (sizeof (mynfs_msg_t) + sub_msg_size);
    memcpy(clientMsg->data, clientSubMsg, sub_msg_size);

    clientMsg->cmd= 6;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg->handle = 0;
    clientMsg->data_length = sub_msg_size;

    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, port, clientMsg, &serverMsg);

    // TODO: obsluga kodow bledow z serwera
    return 0;   //0 = success
}


int mynfs_fstat(char *host, int port, int fd, struct stat *buf)
{
    //brak komunikatu podrzednego
    
    //ogarnianie komunikatu nadrzednego
    mynfs_msg_t *clientMsg;
    clientMsg = (mynfs_msg_t *)malloc (sizeof (mynfs_msg_t));

    clientMsg->cmd= 5;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg->handle = fd;
    clientMsg->data_length = 0;

    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, port, clientMsg, &serverMsg);

    stat *stat_table = (stat *) serverMsg->data;
    memcpy(buf, stat_table, serverMsg->data_length);

    // TODO: obsluga kodow bledow z serwera
    return 0;   //0 = success
}