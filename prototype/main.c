#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define UNUSED(x) (void)(x)

#define FREE(ptr) \
    if (ptr) \
    { \
        free(ptr); \
        ptr = NULL; \
    }

struct image_t
{
    int width, height, comp;
    unsigned char *data;
    char *filename;
};

struct geometry_t
{
    uint16_t width;
    uint16_t height;
    uint8_t depth;
};

xcb_image_t*
create_xcb_image(
    const xcb_setup_t *setup,
    struct geometry_t *geometry)
{
    assert(setup);
    assert(geometry);

    xcb_image_t                     *image;
    uint8_t                         *data;
    uint8_t                          byte_depth;
    int                              i,
                                     byte_count;
    byte_depth = geometry->depth / 8;
    assert(byte_depth == 3);

    byte_count = (geometry->width * geometry->height * byte_depth);
    data = malloc(byte_count);

    for (i = 0; i < byte_count; i += byte_depth)
    {
        data[i]   = 0xff;
        data[i+1] = 0x00;
        data[i+2] = 0xff;
    }

    image = xcb_image_create(
        geometry->width,
        geometry->height,
        XCB_IMAGE_FORMAT_Z_PIXMAP,
        0,
        geometry->depth,
        geometry->depth,
        0,
        setup->image_byte_order,
        XCB_IMAGE_ORDER_MSB_FIRST,
        data,
        byte_count,
        data);

    if (!image)
    {
        printf("Failed to create the xcb image\n");
        return NULL;
    }

    return image;
}

void
set_pixmap_atom(
    xcb_connection_t *c,
    const char *atom_name,
    xcb_window_t window,
    xcb_pixmap_t pixmap_id)
{
    xcb_intern_atom_cookie_t   intern_atom_cookie;
    xcb_intern_atom_reply_t   *intern_atom_reply;
    xcb_generic_error_t       *error;

    intern_atom_cookie = xcb_intern_atom(c, true, strlen(atom_name), atom_name);
    intern_atom_reply = xcb_intern_atom_reply(c, intern_atom_cookie, &error);

    assert(!error);
    assert(intern_atom_reply);

    xcb_get_property_cookie_t        get_property_cookie;
    xcb_get_property_reply_t        *get_property_reply;

    get_property_cookie = xcb_get_property(c, 0, window, intern_atom_reply->atom, XCB_ATOM_PIXMAP, 0, 0);
    get_property_reply = xcb_get_property_reply(c, get_property_cookie, &error);
    assert(!error);
    assert(get_property_reply);

    xcb_void_cookie_t change_property_cookie;
    change_property_cookie = xcb_change_property_checked(
        c,
        XCB_PROP_MODE_REPLACE,
        window,
        intern_atom_reply->atom,
        get_property_reply->type,
        get_property_reply->format,
        1,
        (unsigned char *)&pixmap_id);

    error = xcb_request_check(c, change_property_cookie);
    assert(!error);

    free(intern_atom_reply);
}

struct geometry_t*
get_geometry(
    xcb_connection_t *connection,
    xcb_window_t      window)
{
    xcb_get_geometry_cookie_t    cookie;
    xcb_generic_error_t         *error = NULL;
    xcb_get_geometry_reply_t    *reply = NULL;
    struct geometry_t* result;

    cookie = xcb_get_geometry(connection, window);
    reply = xcb_get_geometry_reply(connection, cookie, &error);
    if (error || !reply)
    {
        return NULL;
    }

    result = malloc(sizeof(struct geometry_t));
    result->width = reply->width;
    result->height = reply->height;
    result->depth = reply->depth;

    FREE(reply);

    return result;
}

void
free_geometry(struct geometry_t** geometry)
{
    if (!geometry || !(*geometry))
    {
        return;
    }

    FREE(*geometry);
}

xcb_pixmap_t
put_image_on_pixmap(
    xcb_connection_t  *c,
    const xcb_screen_t      *s,
    const xcb_window_t       w,
    xcb_image_t       *image)
{
    xcb_pixmap_t pmap;
    uint32_t mask;
    uint32_t values[2];
    xcb_gcontext_t gc;

    pmap = xcb_generate_id(c);
    xcb_create_pixmap(c, 24, pmap, w, image->width, image->height);

    /* create pixmap plot gc */
    mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    values[0] = s->black_pixel;
    values[1] = 0xffffff;

    gc = xcb_generate_id (c);
    xcb_create_gc (c, gc, pmap, mask, values);

    /* put the image into the pixmap */
    xcb_image_put(c, pmap, gc, image, 0, 0, 0);

    return pmap;
}

void
set_window_background_pixmap(
    xcb_connection_t *c,
    xcb_window_t window,
    xcb_pixmap_t pixmap)
{
    xcb_void_cookie_t cookie;
    xcb_generic_error_t *error;

    cookie = xcb_change_window_attributes_checked(
        c, window, XCB_CW_BACK_PIXMAP, &pixmap);

    error = xcb_request_check(c, cookie);
    assert(!error);
}

int
set_wallpaper_from_image(const struct image_t *image)
{
    UNUSED(image);

    xcb_connection_t                *connection;
    const xcb_setup_t               *setup;
    xcb_screen_iterator_t            screen_iterator;
    xcb_screen_t                    *screen;
    xcb_window_t                     root;
    struct geometry_t               *geometry;
    xcb_image_t                     *xcb_image;
    xcb_pixmap_t                     pixmap;

    connection = xcb_connect (NULL, NULL);
    setup = xcb_get_setup(connection);
    screen_iterator = xcb_setup_roots_iterator(setup);  
    screen = screen_iterator.data;
    root = screen->root;

    geometry = get_geometry(connection, root);
    assert(geometry);

    xcb_image = create_xcb_image(setup, geometry);
    assert(xcb_image);

    pixmap = put_image_on_pixmap(connection, screen, root, xcb_image);

    set_pixmap_atom(connection, "_XROOTPMAP_ID", root, pixmap);
    set_pixmap_atom(connection, "ESETROOT_PMAP_ID", root, pixmap);

    set_window_background_pixmap(connection, root, pixmap);

    xcb_clear_area(connection, 1, root, 0, 0,
        geometry->width, geometry->height);

    xcb_flush(connection);

    free_geometry(&geometry);
    xcb_disconnect(connection);

    return 0;
}

struct image_t*
load_image(const char *filename)
{
    int width, height, comp;
    unsigned char *data;
    struct image_t* image;

    data = stbi_load(filename, &width, &height, &comp, 0);
    if (!data)
    {
        return NULL;
    }

    image = malloc(sizeof(struct image_t));
    image->filename = strdup(filename);
    image->width = width;
    image->height = height;
    image->comp = comp;
    image->data = data;

    return image;
}

void
free_image(struct image_t** image)
{
    if (!image || !(*image))
    {
        return;
    }

    FREE((*image)->filename);
    FREE((*image)->data);
    FREE((*image));
}

int main(int argc, char **argv)
{
    char           *filename;
    struct image_t *image;

    filename = (argc > 1)
        ? argv[1]
        : "./Imperius.jpg";

    image = load_image(filename);
    if (!image)
    {
        printf("Failed to load image \"%s\"\n", filename);
        return 1;
    }

    set_wallpaper_from_image(image);

    free_image(&image);

    return 0;
}
