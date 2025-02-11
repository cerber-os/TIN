#include <iostream>  
#include <cstdlib>
 
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
    public: std::vector<std::string> pathnames;
    
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
            std::cout << "Access denied. Wrong username or password " << std::endl;
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

bool check_number(string str) {
    for (int i = 0; i < str.length(); i++)
        if (isdigit(str[i]) == false)
            return false;
    return true;
}

std::pair<int, int> chooseDescriptor(std::vector<std::pair<int, int>> &openedDescriptors, std::vector<std::string> &pathnames)
{   bool correctFdFound = false;
    int fd;
    pair<int, int> chosenFd;
    while(!correctFdFound){
        std::cout << "Choose descriptor: type number"<<std::endl;
        for (int i=0; i<openedDescriptors.size(); i++){
            std::cout << "   " << openedDescriptors[i].first << ") " << pathnames[i] << std::endl;
        }
 
        std::string fdStr;
        std::getline(std::cin, fdStr);
        if(!fdStr.empty() && check_number(fdStr)){
            fd = std::stoi(fdStr);
            chosenFd = findDescriptorsPair(fd, openedDescriptors);
            if(chosenFd.first != -1){
                correctFdFound = true;
            }
            else{
                std::cout << "Cant find your descriptor, try again  ";
            }
        }
        else{
            std::cout << "Given input is not a number"<< std::endl;
        }
    }
    return chosenFd;
}

 
int nfsread(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors, std::vector<std::string> &pathnames)
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
 
    auto fd = chooseDescriptor(openedDescriptors, pathnames);
    if (fd.first == -1)
    {
        std::cout << "Bad file descriptor" << std::endl;
        return -1;
    }
 
    std::cout << "Bytes to read:" << std::endl;
    std::getline(std::cin, scount);
    if(scount.empty() || !check_number(scount)){
        std::cout << "Wrong input" << std::endl;
        return -1;
    }
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
 
void type_login_and_username(Client* client_pointer){
    std::cout << "Login:" << std::endl;
    std::getline(std::cin, client_pointer->Login);
    client_pointer->Password.assign(getpass("Password: "));
}

 
int nfsopen(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors, std::string &login, std::string &password, std::vector<std::string> &pathnames, Client* client_pointer)
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
        if (fd == MYNFS_ACCESS_DENIED){
            std::cout << "Type valid username and password" << std::endl;
            type_login_and_username(client_pointer);
            std::cout << "Try again your command" << std::endl;
        }
    }
    else{
        std::cout << "Success" << std::endl;
        std::cout << "Opened file descriptor: " << fd << std::endl;
        openedDescriptors.push_back(std::make_pair(fd, socketFd));
        pathnames.push_back(path);
    }
 
    return fd;
}
 
 
int nfswrite(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors, std::vector<std::string> &pathnames)
{
    std::string buf;
 
    int16_t count;
 
    if(openedDescriptors.empty())
    {
        std::cout << "No descriptors opened." << std::endl;
        return -1;
    }
 
    auto fd = chooseDescriptor(openedDescriptors, pathnames);
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
 
int nfslseek(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors, std::vector<std::string> &pathnames){
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
 
    auto fd = chooseDescriptor(openedDescriptors, pathnames);
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
    if(soffset.empty() || !check_number(soffset)){
        std::cout << "Wrong input" << std::endl;
        return -1;
    }
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
 
 
int nfsclose(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors, std::vector<std::string> &pathnames)
{
    if(openedDescriptors.empty())
    {
        std::cout << "No descriptors opened." << std::endl;
        return -1;
    }
 
    auto fd = chooseDescriptor(openedDescriptors, pathnames);
    if (fd.first == -1)
    {
        std::cout << "Bad file descriptor" << std::endl;
        return -1;
    }
 
    int retval = mynfs_close(fd.second, fd.first);

    if(retval == -100){
        std::cout << "Success" << std::endl;
        std::vector<std::pair<int, int>>::iterator itr = std::find(openedDescriptors.begin(), openedDescriptors.end(), fd);
        int index = std::distance(openedDescriptors.begin(), itr);
        pathnames.erase(pathnames.begin() + index);
        openedDescriptors.erase(std::remove(openedDescriptors.begin(), openedDescriptors.end(), fd),
                                openedDescriptors.end());
    }
    else{
        std::cout << "Cannot close" << std::endl;
        showErrorMsg(retval);
    }
 
    return 0;
}


int nfsunlink(std::string &host, std::string &login, std::string &password, Client* client_pointer)
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
        if (retval == MYNFS_ACCESS_DENIED){
            std::cout << "Type valid username and password" << std::endl;
            type_login_and_username(client_pointer);
            std::cout << "Try again your command" << std::endl;
        }
    }
    return 0;
}


int nfsfstat(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors, std::vector<std::string> &pathnames)
{
 
    if(openedDescriptors.empty())
    {
        std::cout << "No descriptors opened." << std::endl;
        return -1;
    }
 
    auto fd = chooseDescriptor(openedDescriptors, pathnames);
    if (fd.first == -1)
    {
        std::cout << "Bad file descriptor" << std::endl;
        return -1;
    }
 
    struct stat finfo;
 
    int retval = mynfs_fstat(fd.second, fd.first, &finfo);
    if(retval<0){
        std::cout << "Cannot fstat" << std::endl;
        showErrorMsg(retval);
    }
    else{
        std::cout << "Success" << std::endl;
        std::cout << "   ID of device:                      "<< finfo.st_dev << std::endl;
        std::cout << "   Inode number:                      "<< finfo.st_ino << std::endl;
        std::cout << "   Protection:                        "<< finfo.st_mode << std::endl;
        std::cout << "   Number of hard links:              "<< finfo.st_nlink << std::endl;
        std::cout << "   User ID of owner:                  "<< finfo.st_uid << std::endl;
        std::cout << "   Group ID of owner:                 "<< finfo.st_gid << std::endl;
        std::cout << "   Device ID (if special file):       "<< finfo.st_rdev << std::endl;
        std::cout << "   Total size, in bytes:              "<< finfo.st_size << std::endl;
        std::cout << "   Blocksize for file system I/O:     "<< finfo.st_blksize << std::endl;
        std::cout << "   Number of 512B blocks allocated:   "<< finfo.st_blocks << std::endl;
        char buff[20];
        struct tm * timeinfo;
        timeinfo = localtime (&finfo.st_atime);
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M", timeinfo);
        std::cout << "   Time of last access:               "<< buff << std::endl;
        timeinfo = localtime (&finfo.st_mtime);
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M", timeinfo);
        std::cout << "   Time of last modification:         "<< buff << std::endl;
        timeinfo = localtime (&finfo.st_ctime);
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M", timeinfo);
        std::cout << "   Time of last status change:        "<< buff << std::endl;
    }

    return retval;
}

void showHosts(std::vector<Client> &hosts, Client* clientpointer){
    std::cout<< "Hosts available:"<< std::endl;
    for (std::vector<Client>::const_iterator i = hosts.begin(); i != hosts.end(); ++i)
        std::cout << i->IpPort << ' ';
    std::cout<< "Current host: "<< clientpointer->IpPort << std::endl;
 
}

void changeHost( std::vector<Client> &clients , Client** clientPointer){
 
    showHosts(clients, *clientPointer);
    string index;
    std::cout<< "Give host index to change (indexes start from 0): "<< std::endl;
    std::getline(std::cin, index);
    if(!index.empty() && check_number(index) && clients.size() > std::stoi(index) && std::stoi(index)>=0){
        std::cout<< "Host Changed"<< std::endl;
        std::cout<< "CurrentHost:  "<< (*clientPointer)->IpPort<< std::endl;
        *clientPointer = &clients[std::stoi(index)];
    }
    else{
        std::cout<< "Wrong index"<< std::endl;
    }
}
 
void addHost(std::vector<Client> &clients, Client** clientPointer){
if(clients.size() > 2){
    std::cout<< "Cannot add more hosts,"<< std::endl;
    return;
}
else {
    Client tempClient;
    std::cout << "Server address (IP:PORT):" << std::endl;
    std::getline(std::cin, tempClient.IpPort);
    type_login_and_username(&tempClient);
    clients.push_back(tempClient);
    std::cout<< "Host added"<< std::endl;
    *clientPointer = &clients.back();
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
 
    std::vector<Client> Clients;
    Client client;
    Client *currentClient;  //wskaznik na aktualnego klienta
    std::string choice;
    // std::string currentHost;
    // std::vector<std::string> hosts;
    std::vector<std::pair<int, int>> openedDescriptors;     //para na deskryptor pliku + deskryptor socketa
    std::vector<std::string> pathnames;                     //klient przechowuje tez pathnames                     
    bool exit = false;
    std::cout << "Welcome to MyNFS!\n";
    std::cout << "Server address (IP:PORT):" << std::endl;
    std::getline(std::cin, client.IpPort);
    type_login_and_username(&client);
    Clients.push_back(client);
    client.openedDescriptors = openedDescriptors;
    client.pathnames = pathnames;
    currentClient = &Clients[0];    //aktualnym klientem zostaje pierwszy
 
 
    while (!exit)
    {
 
        std::cout << "Available commands to run:" <<std::endl;
        std::cout<< "open, read, write, lseek, close, unlink, fstat, exit, hostadd, hostchange, show" << std::endl;
        std::getline(std::cin, choice);
        if (choice == "open") nfsopen(currentClient->IpPort, currentClient->openedDescriptors, currentClient->Login, currentClient->Password, currentClient->pathnames, currentClient);
 
        else if (choice == "read") nfsread(currentClient->IpPort, currentClient->openedDescriptors, currentClient->pathnames);
 
        else if (choice == "write") nfswrite(currentClient->IpPort, currentClient->openedDescriptors, currentClient->pathnames);
 
        else if (choice == "lseek") nfslseek(currentClient->IpPort, currentClient->openedDescriptors, currentClient->pathnames);
 
        else if (choice == "close") nfsclose(currentClient->IpPort, currentClient->openedDescriptors, currentClient->pathnames);
 
        else if (choice == "unlink") nfsunlink(currentClient->IpPort, currentClient->Login, currentClient->Password, currentClient);

        else if (choice == "fstat") nfsfstat(currentClient->IpPort, currentClient->openedDescriptors, currentClient->pathnames);
 
        else if (choice == "hostchange") changeHost(Clients, &currentClient);
 
        else if (choice == "hostadd") addHost(Clients, &currentClient);
       
        //else if (choice == "hostRemove") removeHost(Clients);
 
        else if (choice == "show") showHosts(Clients, currentClient);
 
        else if (choice == "exit")
        {
            exit = true;
            std::cout << "End of MYNFS!\n";
        }
 
        else std::cout << "Unknown command. Try again.\n";
    }
    return 0;
}
