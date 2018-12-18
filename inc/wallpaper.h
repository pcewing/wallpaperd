#ifndef WALLPAPER_H
#define WALLPAPER_H

#include "core.h"

struct x11_context_t;

wpd_error
wpd_create_x11_context(struct x11_context_t** result);

wpd_error
wpd_destroy_x11_context(struct x11_context_t** result);

void
wpd_initialize_libs(const struct x11_context_t* x11);

wpd_error
wpd_set_wallpaper(const struct x11_context_t* x11, const char * path);

#endif
