#ifndef WALLPAPER_H
#define WALLPAPER_H

#include "core.h"
#include "data.h"

/**
 * Iterates through each screen, calculates the screen dimensions, selects a
 * random wallpaper from the database with matching dimensions, and sets the
 * wallpaper.
 **/
wpd_error_t
wpd_set_wallpapers(
    const struct wpd_db_t *db);

#endif // WALLPAPER_H

