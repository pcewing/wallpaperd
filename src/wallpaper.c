#include "wallpaper.h"
#include "core.h"
#include "log.h"

// TODO: Get rid of this dependency; we should only print in log.h
#include <stdio.h>

#include <string.h>

#include <xcb/xcb.h>

#define PREFERRED_BYTE_DEPTH 4

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*
struct wpd_image_t
{
    wpd_image_metadata* metadata;
    unsigned char* data;
};
*/

struct wpd_image_metadata_t
{
    int width;
    int height;
    int comp;
    unsigned char *path;
};

struct wpd_image_metadata_array_t
{
    int count;
    struct wpd_image_metadata_t **data;
};

struct wpd_image_t
{
    int x;
    int y;
    unsigned char *data;
};

wpd_error_t
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

wpd_error_t
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

wpd_error_t
get_compatible_images(const struct wpd_image_metadata_array_t *image_metadata_array, 
    const xcb_get_geometry_reply_t *geometry)
{
    int i, count;

    // TODO: Replace this with a more robust criteria system
    
    for (i = 0; i < image_metadata_array->count; ++i)
    {
        LOGTRACE("Checking compatibility of image with dimensions %ux%u",
            image_metadata_array->data[i]->width, image_metadata_array->data[i]->height);

        if (image_metadata_array->data[i]->width == geometry->width
            && image_metadata_array->data[i]->height == geometry->height)
        {
            LOGINFO("Found a compatible image at \"%s\"\n",
                image_metadata_array->data[i]->path);
        }
    }
}

struct wpd_image_metadata_t*
get_image_metadata(const char *filename)
{
    struct wpd_image_metadata_t *image_metadata;
    int width, height, comp;

    stbi_info(filename, &width, &height, &comp);

    image_metadata = malloc(sizeof(struct wpd_image_metadata_t));
    image_metadata->width = width;
    image_metadata->height = height;
    image_metadata->comp = comp;
    image_metadata->path = strdup(filename);

    return image_metadata;
}

wpd_error_t
free_image_metadata(struct wpd_image_metadata_t** image_metadata)
{
    if (!image_metadata || !(*image_metadata))
    {
        return WPD_ERROR_NULL_PARAM;
    }

    FREE((*image_metadata)->path);
    FREE(*image_metadata);

    return WPD_ERROR_SUCCESS;
}

struct wpd_image_metadata_array_t*
create_image_metadata_array(const struct file_enumeration_t *enumeration)
{
    struct wpd_image_metadata_array_t *image_metadata_array;
    int i;

    image_metadata_array = malloc(sizeof(struct wpd_image_metadata_array_t));
    image_metadata_array->count = enumeration->node_count;
    image_metadata_array->data = malloc(enumeration->node_count * sizeof(struct wpd_image_metadata_t*));

    for (i = 0; i < enumeration->node_count; ++i)
    {
        image_metadata_array->data[i] = get_image_metadata(enumeration->nodes[i]->path);
    }

    return image_metadata_array;
}

wpd_error_t
free_image_metadata_array(struct wpd_image_metadata_array_t **image_metadata_array)
{
    int i;

    if (!image_metadata_array || !(*image_metadata_array))
    {
        return WPD_ERROR_NULL_PARAM;
    }

    for (i = 0; i < (*image_metadata_array)->count; ++i)
    {
        free_image_metadata(&((*image_metadata_array)->data[i]));
    }

    FREE((*image_metadata_array)->data);
    FREE(*image_metadata_array);

    return WPD_ERROR_SUCCESS;
}

wpd_error_t
wpd_set_wallpapers(const struct file_enumeration_t *enumeration)
{
    xcb_connection_t *connection;
    const xcb_setup_t *setup;
    xcb_screen_iterator_t screen_iterator;
    struct wpd_image_metadata_array_t *image_metadata_array;
        
    connection = xcb_connect (NULL, NULL);
    setup = xcb_get_setup(connection);

    image_metadata_array = create_image_metadata_array(enumeration);

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
        get_compatible_images(image_metadata_array, reply);

        free(reply);

        xcb_screen_next (&screen_iterator);
    }

    free_image_metadata_array(&image_metadata_array);

    xcb_disconnect (connection);

    return WPD_ERROR_SUCCESS;
}

