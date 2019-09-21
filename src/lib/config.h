#ifndef CONFIG_H
#define CONFIG_H

#include "core.h"
#include <stdbool.h>

/**
 * Represents the configuration options related to rotation;
 **/
struct wpd_rotation_t {
    bool enabled;
    uint32_t frequency;
};

/**
 * Represents the application configuration options;
 **/
struct wpd_config_t {
    uint32_t search_path_count;
    char **search_paths;
    struct wpd_rotation_t rotation;
};

/**
 * Determines the config file path and reads the configuration file from disk.
 *
 * Note: This needs to be manually destroyed by the caller via
 * destroy_config().
 **/
wpd_error_t load_config(struct wpd_config_t **config);

/**
 * Frees all resources associated with the config.
 **/
wpd_error_t destroy_config(struct wpd_config_t **config);

#endif // CONFIG_H
