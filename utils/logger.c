#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "logger.h"

struct nfs_logger {
    int file_fd;
    int stdout_fd;

    enum log_level log_level;
    int color_mode;
};

#define LOGGER_NEW_FORMAT_SIZE      (4096)
#define LOGGER_DATE_SIZE            (64)
#define LOGGER_RESET                "\033[0m"
#define LOGGER_BLUE                 "\033[34m"
#define LOGGER_GREEN                "\033[32m"
#define LOGGER_YELLOW               "\033[33m"
#define LOGGER_RED                  "\033[31m"

const char* logs_prefixes[2][LOG_LEVEL_MAX] = {
    [1] = {
        [LOG_LEVEL_INFO] = LOGGER_GREEN "INFO" LOGGER_RESET,
        [LOG_LEVEL_WARN] = LOGGER_YELLOW "WARN" LOGGER_RESET,
        [LOG_LEVEL_ERROR] = LOGGER_RED "ERR" LOGGER_RESET,
    },
    [0] = {
        [LOG_LEVEL_INFO] = "INFO",
        [LOG_LEVEL_WARN] = "WARN",
        [LOG_LEVEL_ERROR] = "ERR",
    }
};

static void nfs_write_file(int file_fd, int color_mode, int log_level, const char* fmt, va_list ap) {
    if(file_fd < 0)
        return;

    char* new_fmt = malloc(LOGGER_NEW_FORMAT_SIZE);
    char* date_str = malloc(LOGGER_DATE_SIZE);
    if(!new_fmt | !date_str) {
        dprintf(file_fd, "Failed to output log - ENOMEM\n");
        goto err;
    }

    time_t now = time(NULL);
    struct tm* time_now = localtime(&now);
    strftime(date_str, LOGGER_DATE_SIZE, "%c", time_now);

    if(color_mode)
        snprintf(new_fmt, LOGGER_NEW_FORMAT_SIZE, 
                "[" LOGGER_BLUE "%s" LOGGER_RESET "]"
                "[%s] %s\n", date_str, logs_prefixes[color_mode][log_level], fmt);
    else
        snprintf(new_fmt, LOGGER_NEW_FORMAT_SIZE, 
                "[%s][%s] %s\n", date_str, logs_prefixes[color_mode][log_level], fmt); 
    
    vdprintf(file_fd, new_fmt, ap);

err:
    free(new_fmt);
    free(date_str);
}


int nfs_log_open(struct nfs_logger** p_logger, const char* path, enum log_level log_level, int color_mode) {
    *p_logger = malloc(sizeof(struct nfs_logger));
    struct nfs_logger* logger = *p_logger;

    logger->log_level = log_level;
    logger->color_mode = color_mode;
    logger->stdout_fd = fileno(stdout);

    if(path == NULL)
        logger->file_fd = -1;
    else {
        logger->file_fd = open(path, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
        if(logger->file_fd < 0) {
            nfs_log_error(logger, "Failed to open log file (errno: %d)", errno);
            return 1;
        }
    }

    return 0;
}

void nfs_log_info(struct nfs_logger* logger, const char* fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    
    if(logger->log_level > LOG_LEVEL_INFO)
        return;

    nfs_write_file(logger->stdout_fd, logger->color_mode, LOG_LEVEL_INFO, fmt, vargs);
    nfs_write_file(logger->file_fd, 0, LOG_LEVEL_INFO, fmt, vargs);

    va_end(vargs);
}


void nfs_log_warn(struct nfs_logger* logger, const char* fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    
    if(logger->log_level > LOG_LEVEL_WARN)
        return;

    nfs_write_file(logger->stdout_fd, logger->color_mode, LOG_LEVEL_WARN, fmt, vargs);
    nfs_write_file(logger->file_fd, 0, LOG_LEVEL_WARN, fmt, vargs);

    va_end(vargs);
}


void nfs_log_error(struct nfs_logger* logger, const char* fmt, ...) {
    va_list vargs;
    va_start(vargs, fmt);
    
    if(logger->log_level > LOG_LEVEL_ERROR)
        return;

    nfs_write_file(logger->stdout_fd, logger->color_mode, LOG_LEVEL_ERROR, fmt, vargs);
    nfs_write_file(logger->file_fd, 0, LOG_LEVEL_ERROR, fmt, vargs);

    va_end(vargs);
}

void nfs_log_close(struct nfs_logger* logger) {
    fsync(logger->stdout_fd);
    close(logger->file_fd);
    free(logger);
}
