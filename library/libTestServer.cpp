#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>

#include "msgStructs.h"

/*
Prosty serwer testowy wyswietlajacy wszystkie nadchodzace komunikaty. Do sprawdzania poprawnosci biblioteki klienckiej
*/


void run_server(int);

void getDataFromSocket(int new_socket);

int main() {

    run_server(21037);

}

void run_server(int portNumber) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);


    // Creating socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // attach socket on 21037
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(portNumber);


    if (bind(server_fd, (struct sockaddr *) &address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }


    while (true) {
        new_socket = accept(server_fd, (struct sockaddr *) &address,
                            (socklen_t *) &addrlen);
        if (new_socket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        getDataFromSocket(new_socket);
    }
}

void getDataFromSocket(int new_socket) {
    char *buf = new char[4000];
    int bytes = recv(new_socket, buf, 4000, 0);
    if(bytes <= 0 ){
        perror("receive failure");
        exit(EXIT_FAILURE);
    }
    std::cout<<std::endl<<"Otrzymano bajtow: "<<bytes<<std::endl;

    mynfs_msg_t *received_message = (mynfs_msg_t *) buf;
    std::cout<<"CMD: "<< received_message->cmd<<std::endl;
    std::cout<<"Handle: "<< received_message->handle<<std::endl;
    std::cout<<"Data length: "<< received_message->data_length<<std::endl;

    //ten fragment jest tylko do komendy read 
    // mynfs_read_t *inner_message = (mynfs_read_t *) received_message->data;
    // std::cout<<"* Read length: "<< inner_message->length <<std::endl;

    //ten fragment jest tylko do komendy unlink 
    mynfs_unlink_t *inner_message = (mynfs_unlink_t *) received_message->data;
    std::cout<<"* Path length: "<< inner_message->path_length <<std::endl;
    std::cout<<"* Path name: "<< inner_message->name <<std::endl;

    std::cout.flush();
}