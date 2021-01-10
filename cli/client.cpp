#include<iostream>  

#include "../library/fileOperations.h"
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

#include <fcntl.h>      //tutaj sa zdefioniowane flagi otwarcia pliku

using namespace std;

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

std::pair<int, int> chooseDescriptor(std::vector<std::pair<int, int>> &openedDescriptors)
{
    std::cout << "Choose descriptor: ";
    for (auto d : openedDescriptors)
        std::cout << d.first << " ";
    std::cout << std::endl;

    std::string fdStr;
    std::getline(std::cin, fdStr);
    int fd = std::stoi(fdStr);

    std::pair<int, int> chosen_pair;

    for(int i=0; i < openedDescriptors.size(); i++){
        if(openedDescriptors[i].first == fd){
            chosen_pair = openedDescriptors[i];
            break;
        }
        return std::make_pair(-1, -1);
    }

    return chosen_pair;
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
        std::cout << "Bad file descriptor " << fd.first << "." << std::endl;
        return -1;
    }

    std::cout << "Bytes to read:" << std::endl;
    std::getline(std::cin, scount);
    count = static_cast<int16_t>(std::stoi(scount));

    int16_t size = mynfs_read(fd.second, fd.first, pBuf, count);

    std::cout << "Success" << std::endl;
    std::cout << size << " bytes read" << std::endl;

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



int nfsopen(std::string &host, std::vector<std::pair<int, int>> &openedDescriptors)
{
    std::string path;
    uint16_t oflag = 0;
    std::string soflag;
    std::map<std::string, uint16_t>::iterator it;
    std::map<std::string, uint16_t> flags;

    std::cout << "Path to file:" << std::endl;
    std::getline(std::cin, path);
    std::cout << "Oflag/s: work in progress - enter to skip" << std::endl;
    std::cout << "Possible values: RDONLY | WRONLY | RDWR" << std::endl;
    std::cout << "In order to use more flags write them down with whitespaces between them." << std::endl;
    std::getline(std::cin, soflag);

    std::istringstream iss(soflag);
    std::string word;
    while (iss >> word)
    {
        it = flags.find(word);
        if (it != flags.end())
        {
            oflag = O_RDONLY; 
        }
        else
        {
            std::cout << "Unknown flag. Function aborted." << std::endl;
            return -1;
        }
    }
    // TODO: W tej chwili w wywolaniu mynfs_open flagi sa ustawione na sztywno, trzeba bedzie je obsluzyc

    int socketFd;
    int fd = mynfs_open((char*)(host).c_str(), (char*)path.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, &socketFd);
    std::cout << "Success" << std::endl;
    std::cout << "Opened file descriptor: " << fd << std::endl;     // TODO: obsluga bledow
    openedDescriptors.push_back(std::make_pair(fd, socketFd));
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
        std::cout << "Bad file descriptor " << fd.first << "." << std::endl;
        return -1;
    }

    std::cout << "Data to write:" << std::endl;
    std::getline(std::cin, buf);

    char *byteArray = const_cast<char *>(buf.data());
    void *pBuf = byteArray;
    count = static_cast<int16_t>(buf.length());

    int16_t size = mynfs_write(fd.second, fd.first, pBuf, count);
    std::cout << "Success" << std::endl;
    std::cout << size << " bytes written" << std::endl;
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
        std::cout << "Bad file descriptor " << fd.first << "." << std::endl;
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

    std::cout << "Success" << std::endl;
    std::cout << "New offset: " << offset << std::endl;
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
        std::cout << "Bad file descriptor " << fd.first << "." << std::endl;
        return -1;
    }

    mynfs_close(fd.second, fd.first);


    openedDescriptors.erase(std::remove(openedDescriptors.begin(), openedDescriptors.end(), fd),
                            openedDescriptors.end());

    std::cout << "Success" << std::endl;
    return 0;
}
int nfsunlink(std::string &host)
{
    std::string path;
    std::cout << "Path to file:" << std::endl;
    std::getline(std::cin, path);
    mynfs_unlink((char*)(host).c_str(), (char*)path.c_str());
    std::cout << "Success" << std::endl;
    return 0;
}

void showHosts(std::vector<string> &hosts, string &host){
    std::cout<< "Hosts available: ,"<< std::endl;
    for (std::vector<string>::const_iterator i = hosts.begin(); i != hosts.end(); ++i)
        std::cout << *i << ' ';
    std::cout<< "Current host: "<< host << std::endl;

}
void changeHost(std::string &host, std::vector<string> &hosts){

    showHosts(hosts, host);
    string index;
    std::cout<< "Give host index to change (indexes start from 0): "<< std::endl;
    std::getline(std::cin, index);
    if(hosts.size() > std::stoi(index) && std::stoi(index)>=0){
        host = hosts[std::stoi(index)];
        std::cout<< "Host Changed: ,"<< std::endl;
    }
}

void addHost(vector<std::string> &hosts){
if(hosts.size() > 2){
    std::cout<< "Cannot add more hosts,"<< std::endl;
    return;
}
else {
    string temp;
    std::cout<< "Server address (IP:PORT):"<< std::endl;
    std::getline(std::cin, temp);
    hosts.push_back(temp);
    std::cout<< "Host added"<< std::endl;
    return;
}
}
void removeHost(vector<std::string> &hosts){
    if(hosts.size() <= 0){
        std::cout<< "Cannot delete more hosts,"<< std::endl;
        return;
    }
    else {
        string temp;
        std::cout<< "Server address (IP:PORT):"<< std::endl;
        std::getline(std::cin, temp);
        std::vector<string>::iterator it;
        it = find (hosts.begin(), hosts.end(), temp);
        if (it != hosts.end())
        { hosts.erase(std::find(hosts.begin(),hosts.end(),temp));
            std::cout<< "Host deleted"<< std::endl; }
        else{
        std::cout<< "Cannot find given host"<< std::endl;
        }
        return;
    }
}


int main(int argc, char *argv[])
{

    //TO DO 3 wektory deskrytporów dla 3 hostów narazie jest jedna
    std::string choice;
    std::string currentHost;
    std::map<int, std::string> filesIdentificators;
    std::vector<std::string> hosts;
    std::map<std::string, std::vector<int>> hostsWithDescriptors;
    std::vector<std::pair<int, int>> openedDescriptors;     //para na deskryptor pliku + deskryptor socketa
    bool exit = false;
    std::cout << "Welcome to MyNFS!\n";
    std::cout << "Server address (IP:PORT):" << std::endl;
    std::getline(std::cin, currentHost);
    hosts.push_back(currentHost);

    while (!exit)
    {

        std::cout << "Available commands to run:" <<std::endl;
        std::cout<< "open, read, write, lseek, close, unlink, exit, hostchange" << std::endl;
        std::getline(std::cin, choice);
        if (choice == "open") nfsopen(currentHost, openedDescriptors);

        else if (choice == "read") nfsread(currentHost, openedDescriptors);

        else if (choice == "write") nfswrite(currentHost, openedDescriptors);

        else if (choice == "lseek") nfslseek(currentHost, openedDescriptors);

        else if (choice == "close") nfsclose(currentHost, openedDescriptors);

        else if (choice == "unlink") nfsunlink(currentHost);

	    else if (choice == "hostchange") changeHost(currentHost, hosts);

	    else if (choice == "hostAdd") addHost(hosts);
	
	    else if (choice == "hostRemove") removeHost(hosts);

        else if (choice == "show") showHosts(hosts, currentHost);

        else if (choice == "exit")
        {
            exit = true;
            std::cout << "End of MYNFS!\n";
        }

        else std::cout << "Unknown command. Try again.\n";
    }
    return 0;
}
