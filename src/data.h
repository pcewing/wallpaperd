#ifndef DATA_H
#define DATA_H

#include "core.h"

// This type is intentionally opaque
struct wpd_db_t;

struct wpd_wallpaper_entity_t
{
    char *path;
    unsigned int width;
    unsigned int height;
};

struct wpd_wallpaper_result_set_t
{
    unsigned int count;
    struct wpd_wallpaper_entity_t *results;
};

// TODO: Document this
wpd_error_t
initialize_database(
    struct wpd_db_t** db);

// TODO: Document this
wpd_error_t
insert_wallpaper(
    const struct wpd_db_t *db,
    const struct wpd_wallpaper_entity_t *wallpaper);

// TODO: Document this
wpd_error_t
get_wallpapers(
    const struct wpd_db_t *db,
    struct wpd_wallpaper_result_set_t **result_set);

// TODO: Document this
wpd_error_t
get_wallpapers_by_dimensions(
    const struct wpd_db_t *db,
    int width,
    int height,
    struct wpd_wallpaper_result_set_t **result_set);

// TODO: Document this
wpd_error_t
cleanup_database(
        struct wpd_db_t** db);

#endif // DATA_H

