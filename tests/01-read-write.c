#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "fileOperations.h"
#include "tests_config.h"

int main() {
    int socketfd;

    int fd = mynfs_open(SERVER_HOST, TEST_FILE_NAME, O_RDWR | O_CREAT, S_IRWXU, &socketfd);
    assert(fd >= 0);

    char sample_data[] = "test_string";
    int ret = mynfs_write(socketfd, fd, sample_data, sizeof(sample_data));
    assert(ret == sizeof(sample_data));

    mynfs_close(socketfd, fd);

    fd = mynfs_open(SERVER_HOST, TEST_FILE_NAME, O_RDONLY, S_IRWXU, &socketfd);
    assert(fd >= 0);

    char response[sizeof(sample_data)];
    ret = mynfs_read(socketfd, fd, response, sizeof(response));
    assert(ret == sizeof(response));
    assert(0 == memcmp(sample_data, response, sizeof(response)));

    mynfs_close(socketfd, fd);
    mynfs_unlink(SERVER_HOST, TEST_FILE_NAME);
    return 0;
}
