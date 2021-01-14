#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "fileOperations.h"
#include "tests_config.h"

int main() {
    int socketfd;
    int fd = mynfs_open(SERVER_HOST, TEST_FILE_NAME, O_RDWR | O_CREAT | O_MYNFS_LOCK, S_IRWXU, &socketfd);
    assert(fd >= 0);

    // Fail is already locked - should fail
    int secondSocketfd;
    int fd2 = mynfs_open(SERVER_HOST, TEST_FILE_NAME, O_RDWR | O_CREAT | O_MYNFS_LOCK, S_IRWXU, &secondSocketfd);
    assert(fd2 < 0);

    // Release 1st lock
    mynfs_close(socketfd, fd);

    // Now we should be able to lock it again
    fd2 = mynfs_open(SERVER_HOST, TEST_FILE_NAME, O_RDWR | O_CREAT | O_MYNFS_LOCK, S_IRWXU, &secondSocketfd);
    assert(fd2 >= 0);

    mynfs_close(secondSocketfd, fd2);
    mynfs_unlink(SERVER_HOST, TEST_FILE_NAME);
    return 0;
}
