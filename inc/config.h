#ifndef CONFIG_H
#define CONFIG_H

#include "core.h"
#include <stdbool.h>

struct wpd_rotation_t
{
    bool enabled;
    uint32_t frequency;
};

struct wpd_config_t
{
    const char **search_paths;
    struct wpd_rotation_t rotation;
};

wpd_error_t
load_config(
    struct wpd_config_t** config);

#endif // CONFIG_H

