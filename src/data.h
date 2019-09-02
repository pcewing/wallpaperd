#ifndef DATA_H
#define DATA_H

#include "core.h"

/**
 * Represents the database context used to interact with the database. This
 * must be initialized via initialize_database and destroyed via
 * cleanup_database.
 *
 * Note: This is intentionally opaque.
 **/
struct wpd_db_t;

/**
 * Represents a wallpaper entity in the database.
 **/
struct wpd_wallpaper_entity_t
{
    char *path;
    unsigned int width;
    unsigned int height;
};

/**
 * Represents the set of results returned when querying the database.
 **/
struct wpd_wallpaper_result_set_t
{
    unsigned int count;
    struct wpd_wallpaper_entity_t *results;
};

/**
 * Initializes the database object.
 *
 * Note: This needs to be manually destroyed by the caller via
 * cleanup_database().
 **/
wpd_error_t
initialize_database(
    struct wpd_db_t** db);

/**
 * Inserts a wallpaper into the database.
 **/
wpd_error_t
insert_wallpaper(
    const struct wpd_db_t *db,
    const struct wpd_wallpaper_entity_t *wallpaper);

/**
 * Gets the list of all wallpapers in the database.
 **/
wpd_error_t
get_wallpapers(
    const struct wpd_db_t *db,
    struct wpd_wallpaper_result_set_t **result_set);

/**
 * Gets the list of all wallpapers in the database with dimensions that match
 * those specified in the parameters.
 **/
wpd_error_t
get_wallpapers_by_dimensions(
    const struct wpd_db_t *db,
    int width,
    int height,
    struct wpd_wallpaper_result_set_t **result_set);

/**
 * Frees all resources associated with the database.
 **/
wpd_error_t
cleanup_database(
        struct wpd_db_t** db);

#endif // DATA_H

