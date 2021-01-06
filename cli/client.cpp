#include<iostream>  

#include "../library/fileOperations.h"
  
using namespace std; 
  
int main() 
{ 
    cout<<"Halo, z tej strony klient"<<endl; 
    char bufor[2137];
    cout<<mynfs_read(21, bufor, 2137)<<endl;
      
    return 0; 
} 