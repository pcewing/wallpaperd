#ifndef WALLPAPER_H
#define WALLPAPER_H

#include "image.h"

void
set_wallpaper(const struct wpd_image_t * image);

void
wpd_set_wallpaper(const char * path);

#endif
