#include <stdint.h>
#include <cstddef>

struct mynfs_msg_t { 

    union { 

        int64_t handle; 

        int64_t return_value; 

    }; 
    uint64_t cmd; 
    size_t data_length; 
    char data[0]; 
    
};


struct mynfs_open_t{ 

    int oflag; 
    int mode; 
    size_t path_length; 
    char name[0]; 

};


struct mynfs_read_t { 

    size_t length; 

};


struct mynfs_write_t { 

    size_t length; 
    char buffer[0]; 

};


struct mynfs_lseek_t { 

    size_t offset; 
    int whence; 

};


struct mynfs_unlink_t{ 

    size_t path_length; 
    char name[0]; 

};