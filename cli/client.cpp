#include<iostream>  

#include "../library/fileOperations.h"
  
using namespace std; 
  
int main() 
{ 
    cout<<"Oto klient"<<endl; 
    //char bufor[2137];
    // cout<<mynfs_read("127.0.0.1", 21037, 21, bufor, 2137)<< " <- funkcja read zwrocila ilosc bajtow odczytanych"<<endl;
    // cout<<mynfs_unlink("127.0.0.1", 21037, "blabla")<< " <- funkcja unlink zwrocila kod"<<endl;
    // cout<<mynfs_open("127.0.0.1", 21037, "blabla", 8, 8)<< " <- funkcja open zwrocila deksryptor"<<endl;
    cout<<mynfs_write("127.0.0.1", 21037, 50, "blabla", 8)<< " <- funkcja open zwrocila deksryptor"<<endl;
      
    return 0; 
} 