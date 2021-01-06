#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "config_parser.h"
#include "logger.h"

// Maximum size of configuration file
#define CONFIG_BUFFER_SIZE      (64800)


struct config_structure {
    size_t number_of_fields;
    struct config_field* fields;
};

static void remove_end_whitespaces(char* line) {
    char* str = line;
    while(*str) str++;
    --str;
    while(str > line && isspace(*str)) *(str--) = '\0';
}

// Parse one line of configuration file
static int parse_config_line(struct nfs_logger* logger, struct config_structure* cfg, char* line, int line_no) {
    // 1. Skip whitespaces at the beginning of the line
    while(*line != '\0' && isspace(*line))
        line++;
    if(*line == '\0')
        return 0;       // Empty line
    
    // 2. Check if line contains comment
    if(*line == '#')
        return 0;

    // 3. Extract parameter name
    char* name = strsep(&line, "=");
    if(line == NULL) {
        nfs_log_error(logger, "Error parsing config file in line %d: missing value of parameter `%s`", line_no, name);
        return 1;
    }
    remove_end_whitespaces(name);

    // 4. Find entry in config_structure for this field
    struct config_field* field = NULL;
    for(int i = 0; i < cfg->number_of_fields; i++)
        if(!strcasecmp(name, cfg->fields[i].name)) {
            field = &cfg->fields[i];
            break;
        }
    if(field == NULL) {
        nfs_log_error(logger, "Error parsing config file in line %d: unknown config name `%s`", line_no, name);
        return 1;
    }
    if(field->_is_set) {
        nfs_log_error(logger, "Error parsing config file in line %d: redefinition of parameter `%s` value", line_no, name);
        return 1;
    }

    // 5. Skip whitespaces and extract value of field
    while(*line != '\0' && isspace(*line))
        line++;
    if(*line == '\0') {
        nfs_log_error(logger, "Error parsing config file in line %d: empty value for parameter `%s`", line_no, name);
        return 1;
    }
    
    remove_end_whitespaces(line);

    long result; char* str_end; char* str_ptr;
    switch(field->type) {
        case CONFIG_TYPE_BOOL:
            if(!strcasecmp(line, "true"))
                *(int*)field->dst = 1;
            else if(!strcasecmp(line, "false"))
                *(int*)field->dst = 0;
            else {
                nfs_log_error(logger, "Error parsing config file in line %d: "
                    "invalid value for parameter `%s` - expected [true, false], got: `%s`", line_no, name, line);
                return 1;
            }
            break;

        case CONFIG_TYPE_INT:
            result = strtol(line, &str_end, 10);
            if(line == str_end || result > (unsigned int) -1) {
                nfs_log_error(logger, "Error parsing config file in line %d: "
                    "invalid value for parameter `%s` - expected valid 32-bit integer, got: `%s`", line_no, name, line);
                return 1;
            }
            *(int*)field->dst = result;
            break;

        case CONFIG_TYPE_STRING:
            result = strlen(line) + 1;
            str_ptr = calloc(1, result);
            if(str_ptr == NULL) {
                nfs_log_error(logger, "Error parsing config file in line %d: "
                    "failed to parse value for parameter `%s` - ENOMEM", line_no, name);
                return 1;
            }

            strcpy(str_ptr, line);
            *(char**)field->dst = str_ptr;
            break;

        default:
            // Compile-time bug. Just throw an error...
            assert(0);
            break;
    }

    // 6. Mark field as set
    field->_is_set = 1;

    return 0;
}


int parse_config(struct config_field* fields, size_t number_of_fields, char* path) {
    int ret = 1;
    struct nfs_logger* logger;

    struct config_structure cfg = {
        .number_of_fields = number_of_fields,
        .fields = fields
    };

    nfs_log_open(&logger, NULL, LOG_LEVEL_INFO, 1);
    nfs_log_info(logger, "Parsing configuration file %s...", path);

    int fd = open(path, O_RDONLY);
    if(fd < 0) {
        nfs_log_error(logger, "Failed to open config file (%d)", errno);
        ret = 2;
        goto err;
    }

    char* config_buffer = calloc(1, CONFIG_BUFFER_SIZE + 1);
    if(config_buffer == NULL) {
        nfs_log_error(logger, "Failed to allocate memory for config file - ENOMEM");
        goto err_close;
    }

    size_t read_part, total_read = 0;
    while(total_read < CONFIG_BUFFER_SIZE && 
            (read_part = read(fd, config_buffer + total_read, CONFIG_BUFFER_SIZE - total_read)) > 0)
        total_read += read_part;
    
    if(total_read == 0) {
        nfs_log_error(logger, "Failed to read more than zero bytes of config file");
        goto err_free;
    }
    nfs_log_info(logger, "Read config file of size %d bytes", total_read);

    int line_no = 1;
    char* tok, *end = config_buffer;
    while((tok = strsep(&end, "\n")) != NULL)
        if(parse_config_line(logger, &cfg, tok, line_no++))
            goto err_free;

    // Finally, check if all manadory fields are set
    for(int i = 0; i < cfg.number_of_fields; i++)
        if(cfg.fields[i].mandatory && !cfg.fields[i]._is_set) {
            nfs_log_error(logger, "Mandatory field %s is not set in config file", cfg.fields[i].name);
            goto err_free;
        }

    // If we made so far, that's hell of a success
    ret = 0;
err_free:
    free(config_buffer);
err_close:
    close(fd);
err:
    nfs_log_close(logger);
    return ret;
}
