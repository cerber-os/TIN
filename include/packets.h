#ifndef PACKETS_H
#define PACKETS_H

/*
 * Everything releated to communication between server and client
 */

#include <stdint.h>
#include <unistd.h>

enum mynfs_error_code {
    // Critical bugs
    MYNFS_INVALID_CLIENT   = -1000,

    // Non-critical bugs
    MYNFS_INVALID_PACKET   = -500,
    MYNFS_UNKNOWN_COMMAND,
    MYNFS_ALREADY_OPENED,
    MYNFS_OVERLOAD,

    // Others
    MYNFS_CLOSED            = 100,

    // Success
    MYNFS_SUCCESS           = 0,
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
    uint8_t data[0];
};


/*
 * Structures provided in field `data` by client
 */
struct mynfs_open_t {
    int32_t oflag;
    int32_t mode;
    size_t path_length;
    uint8_t name[0];
};

struct mynfs_read_t {
    size_t length;
};

struct mynfs_write_t {
    size_t length;
    uint8_t buffer[0];
};

struct mynfs_lseek_t {
    size_t offset;
    int32_t whence;
};

struct mynfs_unlink_t {
    size_t path_length;
    uint8_t name[0];
};

#endif //PACKETS_H
