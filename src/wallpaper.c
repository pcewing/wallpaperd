#include "wallpaper.h"
#include "core.h"
#include "log.h"

#include <xcb/xcb.h>

#define PREFERRED_BYTE_DEPTH 4

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct x11_context_t;

struct wpd_image_t
{
    int x;
    int y;
    unsigned char *data;
};

wpd_error
wpd_create_x11_context(struct x11_context_t** result)
{
    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_destroy_x11_context(struct x11_context_t** result)
{
    return WPD_ERROR_SUCCESS;
}

void
wpd_initialize_libs(const struct x11_context_t* x11)
{
}

wpd_error
wpd_load_image(const char* path, struct wpd_image_t** result)
{
    int x, y, n;
    unsigned char *data = stbi_load(path, &x, &y, &n, PREFERRED_BYTE_DEPTH);
    if (!data)
    {
        LOGERROR("STBI_LOAD failure");
        return WPD_ERROR_TODO;
    }

    LOGINFO("STBI_LOAD success");

    (*result) = malloc(sizeof(struct wpd_image_t));
    (*result)->x = x;
    (*result)->y = y;
    (*result)->data = data;
    
    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_free_image(struct wpd_image_t** image)
{
    if (!image || !(*image) || !(*image)->data)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    stbi_image_free((*image)->data);
    free(*image);

    return WPD_ERROR_SUCCESS;
}

wpd_error
wpd_set_wallpaper(const struct x11_context_t* x11, const char * path)
{
    xcb_connection_t *connection = xcb_connect (NULL, NULL);
    const xcb_setup_t *setup = xcb_get_setup(connection);

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);  

    // we want the screen at index screenNum of the iterator
    while (1) {
        xcb_screen_next (&iter);

        xcb_screen_t *screen = iter.data;
        LOGINFO("\n");
        LOGINFO("Informations of screen %u:\n", screen->root);
        LOGINFO("  width.........: %u\n", screen->width_in_pixels);
        LOGINFO("  height........: %u\n", screen->height_in_pixels);
        LOGINFO("  white pixel...: %u\n", screen->white_pixel);
        LOGINFO("  black pixel...: %u\n", screen->black_pixel);
        LOGINFO("\n");

        struct wpd_image_t* image;
        TRY(wpd_load_image(path, &image));

        TRY(wpd_free_image(&image));
    }


    xcb_disconnect (connection);

    return WPD_ERROR_SUCCESS;
}
