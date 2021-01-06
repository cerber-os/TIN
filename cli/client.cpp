#include<iostream>  

#include "../library/fileOperations.h"
  
using namespace std; 
  
int main() 
{ 
    cout<<"Oto klient"<<endl; 
    char bufor[2137];
    // cout<<mynfs_read(21, bufor, 2137)<<endl;
    cout<<mynfs_unlink("blabla")<< " <- funkcja unlink zwrocila kod"<<endl;
      
    return 0; 
} 