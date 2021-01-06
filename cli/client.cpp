#include<iostream>  

#include "../library/fileOperations.h"
  
using namespace std; 
  
int main() 
{ 
    cout<<"Halo, z tej strony klient"<<endl; 

    char bufor[20];
    cout<<mynfs_read(1, bufor, 20)<<endl;
      
    return 0; 
} 