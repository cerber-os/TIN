#include "handleSockets.h"

int sendMessageAndGetResponse(char *serverIp, uint16_t port, mynfs_msg_t *clientRequest, mynfs_msg_t **serverResponse) {
    int socketFd;
    struct sockaddr_in serv_addr;

    //Tworzenie socketa
    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error during socket creating" << std::endl;
        return 1;
    }
    std::cout<<"Socket creating succced"<<std::endl;


    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    //Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, serverIp, &serv_addr.sin_addr) <= 0) {
        std::cout << "inet_pton error" << std::endl;
        return 1;
    }

    //Connect socketa
    if (connect(socketFd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error during connection setting" << std::endl;
        return 1;
    }
    std::cout<<"Connect succced"<<std::endl;

    //Wysylanie requesta do serwera
    write(socketFd, clientRequest, sizeof(mynfs_msg_t) + clientRequest->data_length);

    /*
    Ta czesc jest wykomentowana, bo nie mamy mozliwosci narazie przetestowac odpowiedzi serwera

    //Odbieranie response z serwera
    char response[4000];
    read(socketFd, response, 4000);

    (*serverResponse) = (mynfs_msg_t *) response;
    std::cout << "Response is received" << std::endl;
    std::cout << "Command number: " << (*serverResponse)->cmd << std::endl;
    std::cout << "Data length: " << (*serverResponse)->data_length << std::endl;
    std::cout << "Return value: " << (*serverResponse)->return_value << std::endl;
    */


    return 1;
}