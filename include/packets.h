/*
 * Everything releated to communication between server and client
 */

#include <stdint.h>
#include <unistd.h>

enum mynfs_error_code {
    MYNFS_ERROR_STH     = -1000,
    MYNFS_ERROR_ANOTHER,

    MYNFS_SUCCESS       = 0,
};


enum mynfs_commands {
    MYNFS_CMD_OPEN = 0,
    MYNFS_CMD_READ,
    MYNFS_CMD_WRITE,
    MYNFS_CMD_LSEEK,
    MYNFS_CMD_CLOSE,
    MYNFS_CMD_FSTAT,
    MYNFS_CMD_UNLINK,
};


struct mynfs_datagram_t {
    union {
        int64_t handle;
        int64_t return_value;
    };

    uint64_t cmd;
    size_t data_length;
    char data[0];
};


/*
 * Structures provided in field `data` by client
 */
struct mynfs_open_t {
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

struct mynfs_unlink_t {
    size_t path_length;
    char name[0];
};
