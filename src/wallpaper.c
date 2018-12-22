#include "wallpaper.h"
#include "core.h"
#include "log.h"
#include "image.h"

// TODO: Get rid of this dependency; we should only print in log.h
#include <stdio.h>

#include <string.h>
#include <stdbool.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void
print_error(const xcb_generic_error_t *error)
{
    if (error)
    {
        printf("Error:\n");
        printf("(uint8_t) response_type: %u\n", error->response_type);
        printf("(uint8_t) error_code: %u\n", error->error_code);
        printf("(uint16_t) sequence: %u\n", error->sequence);
        printf("(uint32_t) resource_id: %u\n", error->resource_id);
        printf("(uint16_t) minor_code: %u\n", error->minor_code);
        printf("(uint8_t) major_code: %u\n", error->major_code);
        printf("(uint32_t) full_sequence: %u\n\n", error->full_sequence);
    }
}

void
print_geometry(const xcb_get_geometry_reply_t *reply)
{
    if (reply)
    {
        printf("Geometry:\n");
        printf("(uint8_t) response_type: %u\n", reply->response_type);
        printf("(uint8_t) depth: %u\n", reply->depth);
        printf("(uint16_t) sequence: %u\n", reply->sequence);
        printf("(uint32_t) length: %u\n", reply->length);
        printf("(xcb_window_t) root: %u\n", reply->root);
        printf("(int16_t) x: %u\n", reply->x);
        printf("(int16_t) y: %u\n", reply->y);
        printf("(uint16_t) width: %u\n", reply->width);
        printf("(uint16_t) height: %u\n", reply->height);
        printf("(uint16_t) border_width: %u\n\n", reply->border_width);
    }
}

bool
is_image_compatible(
    const struct wpd_image_metadata_t *image,
    const xcb_get_geometry_reply_t *geometry)
{
    LOGTRACE("Checking compatibility of image with dimensions %ux%u",
        image->width, image->height);

    if (image->width == geometry->width && image->height == geometry->height)
    {
        LOGINFO("Found a compatible image at \"%s\"\n",
            image->path);
        return true;
    }

    return false;
}

wpd_error_t
get_compatible_image_index(
    const struct wpd_image_metadata_array_t *image_metadata_array, 
    const xcb_get_geometry_reply_t *geometry,
    int *out)
{
    int i, count, chosen;
    int indices[1024];

    count = 0;
    for (i = 0; i < image_metadata_array->count; ++i)
    {
        if (is_image_compatible(image_metadata_array->data[i], geometry))
        {
            indices[count] = i;
            count++;
        }
    }

    if (count == 0)
    {
        return WPD_ERROR_NO_COMPATIBLE_IMAGE;
    }

    (*out) = indices[wpd_rand() % count];
    return WPD_ERROR_SUCCESS;
}

wpd_error_t
wpd_set_wallpaper(const struct wpd_image_metadata_t *image_metadata)
{
    int width,
        height,
        comp;
    unsigned char *data;
    
    data = stbi_load(image_metadata->path, &width, &height, &comp, 0);
    if (!data)
    {
        return WPD_ERROR_TODO;
    }

    // TODO: Create an xcb_image_t, draw it to a pixmap, then swap out the root
    // pixmap properties
}

wpd_error_t
wpd_set_wallpapers(const struct wpd_image_metadata_array_t *image_metadata_array)
{
    xcb_connection_t *connection;
    const xcb_setup_t *setup;
    xcb_screen_iterator_t screen_iterator;
        
    connection = xcb_connect (NULL, NULL);
    setup = xcb_get_setup(connection);

    screen_iterator = xcb_setup_roots_iterator(setup);  
    while (screen_iterator.rem) {
        xcb_get_geometry_cookie_t    cookie;
        xcb_generic_error_t         *error = NULL;
        xcb_get_geometry_reply_t    *reply = NULL;

        // Get the geometry of the root window of this screen
        cookie = xcb_get_geometry(connection, screen_iterator.data->root);
        reply = xcb_get_geometry_reply(connection, cookie, &error);

        if (error)
        {
            print_error(error);
            return WPD_ERROR_TODO;
        }

        LOGINFO("Finding compatible images for screen with dimensions %ux%u\n",
            reply->width, reply->height);

        print_geometry(reply);

        // Get compatible images
        int index;
        TRY(get_compatible_image_index(image_metadata_array, reply, &index));
        //wpd_set_wallpaper(image_metadata_array->data[index]);

        free(reply);

        xcb_screen_next(&screen_iterator);
    }

    xcb_disconnect(connection);

    return WPD_ERROR_SUCCESS;
}

