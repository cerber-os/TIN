#include "logger.h"
#include "config_parser.h"

struct nfs_logger* logger;

int port_number, colored_logs;
char* hostname, * nfs_path;

struct config_field fields[] = {
    {.name = "PORT", .type = CONFIG_TYPE_INT, .dst = &port_number, .mandatory = 1},
    {.name = "HOSTNAME", .type = CONFIG_TYPE_STRING, .dst = &hostname},
    {.name = "NFS_PATH", .type = CONFIG_TYPE_STRING, .dst = &nfs_path},
    {.name = "COLORED_LOGS", .type = CONFIG_TYPE_BOOL, .dst = &colored_logs},
};

int main() {
    nfs_log_open(&logger, "/tmp/test.txt", LOG_LEVEL_INFO, 1);
    nfs_log_info(logger, "Server version V0.1 starting");

    parse_config(fields, 4, "server/example.cfg");
    nfs_log_info(logger, "Selected port: %d", port_number);
}
