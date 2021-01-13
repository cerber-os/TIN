#include<iostream>  
 
#include "../library/fileOperations.h"
#include "../include/packets.h"
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
 
#include <fcntl.h> 
 
using namespace std;
 
class Client{
    public: std::string IpPort;
    public: std::string Login;
    public: std::string Password;
    public: std::vector<std::pair<int, int>> openedDescriptors;
    
    Client(){
            IpPort ="";
            Login ="";
            Password="";
    
        }
    public: std::string getIpPort(){
            return IpPort;
        }
    public: std::string getLogin(){
            return Login;
        }
    public: std::string getPassword(){
            return Password;
        }
    public: std::vector<std::pair<int, int>> getDescriptors(){
            return openedDescriptors;
        }
};

int getPort(string host){
    std::size_t pos = host.find_last_of(':');
    string port = host.substr(pos + 1, host.length() - 1);
    return std::stoi( port );
}
 
 
string getIp (string host){
    std::size_t pos = host.find_last_of(':');
    string ip = host.substr(0, pos);
    return ip;
}
 
 
void showErrorMsg(int errorNumber){
    switch( errorNumber ) {
        case MYNFS_INVALID_CLIENT:
            std::cout << "MYNFS_INVALID_CLIENT " << std::endl;
            break;
 
        case MYNFS_INVALID_PACKET:
            std::cout << "MYNFS_INVALID_PACKET " << std::endl;
            break;
 
        case MYNFS_UNKNOWN_COMMAND:
            std::cout << "MYNFS_UNKNOWN_COMMAND " << std::endl;
            break;
        case MYNFS_ALREADY_OPENED:
            std::cout << "MYNFS_ALREADY_OPENED " << std::endl;
            break;
 
        case MYNFS_OVERLOAD:
            std::cout << "MYNFS_OVERLOAD " << std::endl;
            break;
 
        case MYNFS_INVALID_PATH:
            std::cout << "MYNFS_INVALID_PATH " << std::endl;
            break;
 
        case MYNFS_ALREADY_LOCKED:
            std::cout << "MYNFS_ALREADY_LOCKED " << std::endl;
            break;
        case MYNFS_ACCESS_DENIED:
            std::cout << "MYNFS_ALREADY_LOCKED " << std::endl;
            break;
    }
    return;
}
 
 
std::pair<int, int> findDescriptorsPair(int fd, std::vector<std::pair<int, int>> &openedDescriptors){
    for(auto desc : openedDescriptors){
        if(desc.first == fd) return desc;
    }
    return std::make_pair(-1, -1);
}

std::pair<int, int> chooseDescriptor(std::vector<std::pair<int, int>> &openedDescriptors)
{   bool correctFdFound = false;
    int fd;
    pair<int, int> chosenFd;
    while(!correctFdFound){
        std::cout << "Choose descriptor from: ";
        for (auto d : openedDescriptors)
            std::cout << d.first << " ";
        std::cout << std::endl;
 
        std::string fdStr;
        std::getline(std::cin, fdStr);
        fd = std::stoi(fdStr);
        chosenFd = findDescriptorsPair(fd, openedDescriptors);

        if(chosenFd.first != -1){
            correctFdFound = true;
        }
        else{
            std::cout << "Cant find your descriptor, try again  ";
        }
    }
    return chosenFd;
}
 
 
int nfsread(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors)
{
    int16_t count;
    std::string scount;
    unsigned char buf[4096];
    void *pBuf;
    pBuf = &buf[0];
 
    if(openedDescriptors.empty())
    {
        std::cout << "No descriptors opened." << std::endl;
        return -1;
    }
 
    auto fd = chooseDescriptor(openedDescriptors);
    if (fd.first == -1)
    {
        std::cout << "Bad file descriptor" << std::endl;
        return -1;
    }
 
    std::cout << "Bytes to read:" << std::endl;
    std::getline(std::cin, scount);
    count = static_cast<int16_t>(std::stoi(scount));
 
    int16_t size = mynfs_read(fd.second, fd.first, pBuf, count);
 
    if(size < 0){
        std::cout << "Cannot read" << std::endl;      
        showErrorMsg(size);
    }
    else{
        std::cout << "Success" << std::endl;
        std::cout << size << " bytes read" << std::endl;
    }

 
    int16_t i = 0;
    while (i < size)
    {
        std::cout << buf[i];
        if (i == 100) std::cout << std::endl;
        i++;
    }
 
    std::cout << std::endl;
    return size;
}
uint16_t orWord(uint16_t value, uint16_t word)
{
    return value | word;
}
 
 
int nfsopen(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors, std::string &login, std::string &password)
{
    std::string path;
    uint16_t oflag = 0;
    std::string soflag;
    std::map<std::string, uint16_t>::iterator it;
    std::map<std::string, uint16_t> flags;
 
    flags["RDONLY"] = 0;
    flags["WRONLY"] = 1;
    flags["RDWR"] = 2;
    flags["CREAT"] = 64;
    flags["LOCK"] = 32768;
 
    std::cout << "Path to file:" << std::endl;
    std::getline(std::cin, path);

    std::string whole_path = login + ":" + password + "@" + path;
    std::cout<<"[DEBUG] Tak wyglada caly wysylany path: "<<whole_path<<std::endl;

    std::cout << "Oflag/s:" << std::endl;
    std::cout << "Choose flag: RDONLY | WRONLY | RDWR | CREAT | LOCK" << std::endl;
    std::cout << "In order to use more flags write them one after another with spaces." << std::endl;
    std::getline(std::cin, soflag);
 
 
    std::istringstream iss(soflag);
    std::string word;
    while (iss >> word)
    {
        it = flags.find(word);
        if (it != flags.end())
        {
            oflag = orWord(oflag, it->second);
        }
        else
        {
            std::cout << "Unknown flag. Function aborted." << std::endl;
            return -1;
        }
    }
 
    int socketFd;
    int fd = mynfs_open((char*)(host).c_str(), (char*)whole_path.c_str(), oflag, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, &socketFd);
    if(fd<0){
        std::cout << "Cannot open" << std::endl;
        showErrorMsg(fd);
    }
    else{
        std::cout << "Success" << std::endl;
        std::cout << "Opened file descriptor: " << fd << std::endl;
        openedDescriptors.push_back(std::make_pair(fd, socketFd));
    }
 
    return fd;
}
 
 
int nfswrite(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors)
{
    std::string buf;
 
    int16_t count;
 
    if(openedDescriptors.empty())
    {
        std::cout << "No descriptors opened." << std::endl;
        return -1;
    }
 
    auto fd = chooseDescriptor(openedDescriptors);
    if (fd.first == -1)
    {
        std::cout << "Bad file descriptor" << std::endl;
        return -1;
    }
 
    std::cout << "Data to write:" << std::endl;
    std::getline(std::cin, buf);
 
    char *byteArray = const_cast<char *>(buf.data());
    void *pBuf = byteArray;
    count = static_cast<int16_t>(buf.length());
 
    int16_t size = mynfs_write(fd.second, fd.first, pBuf, count);
    if(size<0){
        std::cout << "Cannot write" << std::endl;
        showErrorMsg(size);
    }
    else{
        std::cout << "Success" << std::endl;
        std::cout << size << " bytes written" << std::endl;
    }
    return size;
}
 
int nfslseek(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors){
    std::string soffset;
    int32_t offset;
    std::string swhence;
    uint8_t whence;
    std::map<std::string, uint8_t>::iterator it;
    std::map<std::string, uint8_t> flags;
 
    if(openedDescriptors.empty())
    {
        std::cout << "No descriptors opened." << std::endl;
        return -1;
    }
 
    auto fd = chooseDescriptor(openedDescriptors);
    if (fd.first == -1)
    {
        std::cout << "Bad file descriptor" << std::endl;
        return -1;
    }
 
    flags["SEEK_SET"] = 0;
    flags["SEEK_CUR"] = 1;
    flags["SEEK_END"] = 2;
 
    std::cout << "Whence:" << std::endl;
    std::cout << "Possible values: SEEK_SET | SEEK_CUR | SEEK_END" << std::endl;
    std::getline(std::cin, swhence);
    std::cout << "Offset:" << std::endl;
    std::getline(std::cin, soffset);
    offset = static_cast<int32_t>(std::stoi(soffset));
 
    it = flags.find(swhence);
    if (it != flags.end())
    {
        whence = it->second;
    }
    else
    {
        std::cout << "Wrong whence. Function aborted." << std::endl;
        return -1;
    }
 
    offset = mynfs_lseek(fd.second, fd.first, offset, whence);
    if(offset<0){
        std::cout << "Cannot lseek" << std::endl;
        showErrorMsg(offset);
    }
    else{
        std::cout << "Success" << std::endl;
        std::cout << "New offset: " << offset << std::endl;
    }

    return offset;
}
 
 
int nfsclose(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors)
{
    if(openedDescriptors.empty())
    {
        std::cout << "No descriptors opened." << std::endl;
        return -1;
    }
 
    auto fd = chooseDescriptor(openedDescriptors);
    if (fd.first == -1)
    {
        std::cout << "Bad file descriptor" << std::endl;
        return -1;
    }
 
    int retval = mynfs_close(fd.second, fd.first);

    if(retval == -100){
        std::cout << "Success" << std::endl;
        openedDescriptors.erase(std::remove(openedDescriptors.begin(), openedDescriptors.end(), fd),
                                openedDescriptors.end());
    }
    else{
        std::cout << "Cannot close" << std::endl;
        showErrorMsg(retval);
    }
 
    return 0;
}


int nfsunlink(std::string &host, std::string &login, std::string &password)
{
    std::string path;
    std::cout << "Path to file:" << std::endl;
    std::getline(std::cin, path);

    std::string whole_path = login + ":" + password + "@" + path;

    int retval = mynfs_unlink((char*)(host).c_str(), (char*)whole_path.c_str());
    if(retval == -100){
        std::cout << "Success" << std::endl;
    }
    else{
        std::cout << "Cannot unlink" << std::endl;
        showErrorMsg(retval);
    }
    return 0;
}
 

void showHosts(std::vector<Client> &hosts, Client &clientIterator){
    std::cout<< "Hosts available: ,"<< std::endl;
    for (std::vector<Client>::const_iterator i = hosts.begin(); i != hosts.end(); ++i)
        std::cout << i->IpPort << ' ';
    std::cout<< "Current host: "<< clientIterator.IpPort << std::endl;
 
}

void changeHost( std::vector<Client> &clients , Client &clientIterator){
 
    showHosts(clients, clientIterator);
    string index;
    std::cout<< "Give host index to change (indexes start from 0): "<< std::endl;
    std::getline(std::cin, index);
    if(clients.size() > std::stoi(index) && std::stoi(index)>=0){
        clientIterator = clients[std::stoi(index)];
        std::cout<< "Host Changed"<< std::endl;
        std::cout<< "CurrentHost:  "<< clientIterator.IpPort<< std::endl;
    }
}
 
void addHost(std::vector<Client> &clients){
if(clients.size() > 2){
    std::cout<< "Cannot add more hosts,"<< std::endl;
    return;
}
else {
    Client tempClient;
    std::cout << "Server address (IP:PORT):" << std::endl;
    std::getline(std::cin, tempClient.IpPort);
    std::cout << "Login:" << std::endl;
    std::getline(std::cin, tempClient.Login);
    std::cout << "Password:" << std::endl;
    std::getline(std::cin, tempClient.Password);
    clients.push_back(tempClient);
    std::cout<< "Host added"<< std::endl;
    return;
}
}
/*void removeHost(std::vector<Client> &clients){
    if(clients.size() <= 0){
        std::cout<< "Cannot delete more hosts,"<< std::endl;
        return;
    }
    else {
        string temp;
        std::cout<< "Server address (IP:PORT):"<< std::endl;
        std::getline(std::cin, temp);
        std::vector<Client>::iterator it;
        it = find (clients.begin()->IpPort, clients.end() ->IpPort, temp);
        if (it != hosts.end())
        { hosts.erase(std::find(hosts.begin(),hosts.end(),temp));
            std::cout<< "Host deleted"<< std::endl; }
        else{
        std::cout<< "Cannot find given host"<< std::endl;
        }
        return;
    }
}*/
 
 
 
int main(int argc, char *argv[])
{
 
    //TO DO 3 wektory deskrytporów dla 3 hostów narazie jest jedna
    std::vector<Client> Clients;
    Client client;
    Client *currentClient;  //wskaznik na aktualnego klienta
    std::string choice;
    std::string currentHost;
    std::vector<Client>::iterator currentClientIterator;
    std::vector<std::string> hosts;
    std::vector<std::pair<int, int>> openedDescriptors;     //para na deskryptor pliku + deskryptor socketa
    bool exit = false;
    std::cout << "Welcome to MyNFS!\n";
    std::cout << "Server address (IP:PORT):" << std::endl;
    std::getline(std::cin, client.IpPort);
    std::cout << "Login:" << std::endl;
    std::getline(std::cin, client.Login);
    std::cout << "Password:" << std::endl;
    std::getline(std::cin, client.Password);
    Clients.push_back(client);
    client.openedDescriptors = openedDescriptors;
    currentClient = &Clients[0];    //aktualnym klientem zostaje pierwszy
 
 
    while (!exit)
    {
 
        std::cout << "Available commands to run:" <<std::endl;
        std::cout<< "open, read, write, lseek, close, unlink, exit, hostchange" << std::endl;
        std::getline(std::cin, choice);
        if (choice == "open") nfsopen(currentClient->IpPort, currentClient->openedDescriptors, currentClient->Login, currentClient->Password);
 
        else if (choice == "read") nfsread(currentClient->IpPort, currentClient->openedDescriptors);
 
        else if (choice == "write") nfswrite(currentClient->IpPort, currentClient->openedDescriptors);
 
        else if (choice == "lseek") nfslseek(currentClient->IpPort, currentClient->openedDescriptors);
 
        else if (choice == "close") nfsclose(currentClient->IpPort, currentClient->openedDescriptors);
 
        else if (choice == "unlink") nfsunlink(currentClient->IpPort, currentClient->Login, currentClient->Password);
 
            else if (choice == "hostchange") changeHost(Clients, *currentClient);
 
            else if (choice == "hostAdd") addHost(Clients);
       
            //else if (choice == "hostRemove") removeHost(Clients);
 
        else if (choice == "show") showHosts(Clients, *currentClient);
 
        else if (choice == "exit")
        {
            exit = true;
            std::cout << "End of MYNFS!\n";
        }
 
        else std::cout << "Unknown command. Try again.\n";
    }
    return 0;
}
