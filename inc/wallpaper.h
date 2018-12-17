#ifndef WALLPAPER_H
#define WALLPAPER_H

extern struct x11_context_t;

int
create_x11_context(struct x11_context_t** result);

void
destroy_x11_context(struct x11_context_t** result);

void
initialize_imlib(const struct x11_context_t* x11);

int
wpd_set_wallpaper(const struct x11_context_t* x11, const char * path);

#endif
