#include "fileOperations.h"


//"należy zaimplementować co najmniej następujące tryby otwarcia pliku: O_RDONLY, O_WRONLY, O_RDWR, O_CREAT"
int mynfs_open(const char *pathname, int flags, mode_t mode){
    return 1;
}

int mynfs_read(int fd, void *buf, size_t count)
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

    char *host = "127.0.0.1";
    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, 21037, clientMsg, &serverMsg);

    int len = 420;
    /* ta czesc jest zakomentowana, bo nie mamy narazie odpowiedzi serwera
    int len = ntohl(serverMsg->data_length);
    memcpy(buf, serverMsg->data, len);
    */

    return len;
}

ssize_t mynfs_write(int fd, const void *buf, size_t count)
{
    return 1;
}

off_t mynfs_lseek(int fd, off_t offset, int whence){
    return 1;
}

int mynfs_close(int fd)
{
    return 1;
}


int mynfs_unlink(const char *pathname)
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

    char *host = "127.0.0.1";
    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, 21037, clientMsg, &serverMsg);

    /* ta czesc jest zakomentowana, bo nie mamy narazie odpowiedzi serwera
    int len = ntohl(serverMsg->data_length);
    memcpy(buf, serverMsg->data, len);
    */

    return 0;   //0 = success
}


int mynfs_fstat(int fd, struct stat *buf)
{
    //brak komunikatu podrzednego
    
    //ogarnianie komunikatu nadrzednego
    mynfs_msg_t *clientMsg;
    clientMsg = (mynfs_msg_t *)malloc (sizeof (mynfs_msg_t));

    clientMsg->cmd= 5;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg->handle = fd;
    clientMsg->data_length = 0;

    char *host = "127.0.0.1";
    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, 21037, clientMsg, &serverMsg);

    /* ta czesc jest zakomentowana, bo nie mamy narazie odpowiedzi serwera
    int len = ntohl(serverMsg->data_length);
    memcpy(buf, serverMsg->data, len);
    trzeba bedzie obsluzyc wczytywanie tablicy stat
    */

    return 0;   //0 = success
}