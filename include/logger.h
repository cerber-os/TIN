/*
 * Library for printing logs to stdout and log file
 * 
 * Logs are of form:
 *  [Mon Dec  11 17:19:05 2021][WARN] This is an example message
 * 
 * Example usage:
 *  struct nfs_logger* logger;
 *  nfs_log_open(&logger, "/tmp/test.txt", LOG_LEVEL_INFO, 1);
 *  nfs_log_warn(logger, "This is an example %s", "message");
 */


struct nfs_logger;


enum log_level {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,

    LOG_LEVEL_MAX
};

/*
 * nfs_log_open - creates a new logger
 *      p_logger - address where the reference to newly created logger should be saved
 *      path - name of file to which logs should be saved (if NULL, logs are only printed to stdout)
 *      log_level - minimum message level required to be printed or saved to log file
 *      color_mode - whether to use colorful output on stdout or stay with boring black and wait
 *  returns 1, when log file couldn't be opened
 */
int nfs_log_open(struct nfs_logger** p_logger, const char* path, enum log_level log_level, int color_mode);

/*
 * nfs_log_debug - print DEBUG message
 */
void nfs_log_debug(struct nfs_logger*, const char* fmt, ...);

/*
 * nfs_log_info - print INFO message
 */
void nfs_log_info(struct nfs_logger*, const char* fmt, ...);

/*
 * nfs_log_warn - print WARNING message
 */
void nfs_log_warn(struct nfs_logger*, const char* fmt, ...);

/*
 * nfs_log_error - print ERROR message
 */
void nfs_log_error(struct nfs_logger*, const char* fmt, ...);

/*
 * nfs_log_close - flush stdout, close log file and release internal structures
 */
void nfs_log_close(struct nfs_logger*);
