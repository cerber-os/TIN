#include "handleSockets.h"

int createSocket(char *serverIp, uint16_t port) {
    int socketFd;
    struct sockaddr_in serv_addr;

    //Tworzenie socketa
    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error during socket creating" << std::endl;
        return 1;
    }
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    //Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, serverIp, &serv_addr.sin_addr) <= 0) {
        std::cout << "Error in inet_pton: probably invalid ip" << std::endl;
        return -1;
    }

    //Connect socketa
    if (connect(socketFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error during connection setting" << std::endl;
        return -1;
    }
    std::cout<<"Connection with server established"<<std::endl;
    return socketFd;
}

int sendAndGetResponse(int socketFd, mynfs_datagram_t *clientRequest, mynfs_datagram_t **serverResponse){
    int rv;

    //Wysylanie requesta do serwera
    if(write(socketFd, clientRequest, sizeof(mynfs_datagram_t) + clientRequest->data_length) == -1){
        std::cerr << "Failed to send request to server" << std::endl;
        return -1;
    }
    std::cout<<"Request sent"<<std::endl;

    //Odbieranie response z serwera
    char response[MAX_BUF];
    size_t header_size = sizeof(mynfs_datagram_t);

    //Wczytywanie samego naglowka
    bzero(response, sizeof(response));
    int recieved_header = 0;
    while(recieved_header < header_size)
    {
        rv = read(socketFd, response + recieved_header, header_size - recieved_header);
        if(rv == -1){
            std::cerr << "Failed to read "<<header_size - recieved_header<<" bytes of header from server. Trying again..." << std::endl;
            continue;
        }
        else if(rv == 0){
            std::cerr << "Read didn't get any data. Server probably disconnected" << std::endl;
            return -1;
        }
        recieved_header += rv;
        std::cout<<"* Get "<<recieved_header<<" bytes of header data"<<std::endl;
    }

    (*serverResponse) = (mynfs_datagram_t *) response;
    size_t submsg_size = (*serverResponse)->data_length;

    int recieved_submsg = 0;
    while(recieved_submsg < submsg_size)
    {
        rv = read(socketFd, response + header_size + recieved_submsg, submsg_size - recieved_submsg);
        if(rv == -1){
            std::cerr << "Failed to read "<<submsg_size - recieved_submsg<<" bytes of submessage from server. Trying again..." << std::endl;
            continue;
        }
        else if(rv == 0){
            std::cerr << "Read didn't get any data. Server probably disconnected" << std::endl;
            return -1;
        }
        recieved_submsg += rv;
        std::cout<<"* Get "<<recieved_submsg<<" bytes of submessage data"<<std::endl;
    }

    std::cout << "Response is received ("<<recieved_header + recieved_submsg<<" bytes)" << std::endl;
    std::cout << "* Return value: " << (*serverResponse)->return_value << std::endl;
    std::cout << "* Command number: " << (*serverResponse)->cmd << std::endl;
    std::cout << "* Data length: " << submsg_size << std::endl;

    return recieved_header + recieved_submsg;
}

int closeSocket(int socketFd){
    return close(socketFd);
}
