/*
 * Library parsing simple configuration files of form `key = value` of the specified
 *  form.
 * 
 * Example config file:
 *  # This is comment
 *  PORT = 1234
 *  HOSTNAME = abcd
 * 
 * Example C program usage:
 *  struct config_field fields[] = {
 *      {.name = "PORT", .type = CONFIG_TYPE_INT, .dst = &port_number},
 *      {.name = "HOSTNAME", .type = CONFIG_TYPE_STRING, .dst = &hostname},
 *  };
 *  parse_config(fields, 2, "path_to_config_file");
 */
#include <unistd.h>

/*
 * List of supported value types:
 *  - BOOL - true/false
 *  - INT - decimal number
 *  - STRING - string not starting or ending with whistespaces
 */
enum value_type {
    CONFIG_TYPE_BOOL = 1,
    CONFIG_TYPE_INT,
    CONFIG_TYPE_STRING,
};


/*
 * Represenation of one config field:
 *  - name - the name of key
 *  - type - the type of value
 *  - dst - pointer at which processed value will be stored
 *  - mandatory - whether field has to be provided in the config file 
 */
struct config_field {
    char* name;
    enum value_type type;
    void* dst;
    int mandatory;

    // Only for internal use - must be set to 0
    int _is_set;
};


/*
 * parse_config - Parse given configuration file
 *  - fields - array of config_field structures representing each supported field
 *  - number_of_fields - number of elements in `fields` array
 *  - path - name of configuration file to process
 * @returns: 0 on success, 1 on file format error, 2 if file doesn't exist
 */
int parse_config(struct config_field* fields, size_t number_of_fields, char* path);
