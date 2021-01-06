#include "fileOperations.h"


//"należy zaimplementować co najmniej następujące tryby otwarcia pliku: O_RDONLY, O_WRONLY, O_RDWR, O_CREAT"
int mynfs_open(const char *pathname, int flags, mode_t mode){
    return 1;
}

int mynfs_read(int fd, void *buf, size_t count)
{
    mynfs_msg_t clientMsg;
    mynfs_read_t clientSubMsg;

    clientSubMsg.length = count;
    char data[sizeof(mynfs_read_t)];

    memcpy(data, &clientSubMsg, sizeof(mynfs_read_t));

    clientMsg.cmd= 345;     //TODO: zamienic liczby na komendy zdefiniowane w ../include
    clientMsg.handle = htonl(fd);       //TODO: sprawdzic czy htonl jest tu wszędzie potrzebne
    clientMsg.data_length = htonl(sizeof(mynfs_read_t));
    // clientMsg.data = new char[sizeof(mynfs_read_t)];
    // clientMsg.data = &data;     //TODO: zapomnialem jak to w C++ sie robi, zaraz to poprawie

    char *host = "127.0.0.1";
    mynfs_msg_t *serverMsg;
    sendMessageAndGetResponse(host, 21037, &clientMsg, &serverMsg);

    int len = 10;
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
    return 1;
}

int mynfs_fstat(int fd, struct stat *buf)
{
    return 1;
}