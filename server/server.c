#include "logger.h"

struct nfs_logger* logger;

int main() {
    nfs_log_open(&logger, "/tmp/test.txt", LOG_LEVEL_INFO, 1);
    nfs_log_info(logger, "TEST TEST %d", 1);
    nfs_log_warn(logger, "TEST TEST %d", 1);
    nfs_log_error(logger, "TEST TEST %d", 1);
}
