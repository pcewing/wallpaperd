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


uint32_t
wpd_min(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
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
select_random_wallpaper(const struct wpd_db_t *db, int width, int height,
    char **result)
{
    struct wpd_wallpaper_result_set_t *wallpapers;
    wpd_error_t error;
    unsigned int i;
    int index;
    char *image_path = NULL;

    error = get_wallpapers_by_dimensions(db, width, height, &wallpapers);
    if (error)
    {
        return error;
    }

    index = rand() % wallpapers->count;
    image_path = strdup(wallpapers->results[index].path);

    for (i = 0; i < wallpapers->count; ++i)
    {
        free(wallpapers->results[i].path);
    }
    free(wallpapers->results);
    free(wallpapers);

    *result = image_path;
    return WPD_ERROR_SUCCESS;
}

/*
 * Rewrite this; it's taken from:
 * http://vincentsanders.blogspot.com/2010/04/xcb-programming-is-hard.html
 */
static xcb_format_t*
wpd_find_format (xcb_connection_t *c, uint8_t depth, uint8_t bpp)
{
    const xcb_setup_t *setup = xcb_get_setup(c);
    xcb_format_t *fmt = xcb_setup_pixmap_formats(setup);
    xcb_format_t *fmtend = fmt + xcb_setup_pixmap_formats_length(setup);
    for(; fmt != fmtend; ++fmt)
    {
        if((fmt->depth == depth) && (fmt->bits_per_pixel == bpp))
        {
            return fmt;
        }
    }
    return 0;
}

void
wpd_get_xcb_image_band_data(
    const struct wpd_image_t *image,
    int                       row_index,
    int                       row_count,
    unsigned char            *data)
{
    unsigned char *image_data_itr;

    int pixels_to_copy = image->width * row_count;

    image_data_itr = image->data + (row_index * image->width * image->bytes_per_pixel);
    while (pixels_to_copy > 0)
    {
        data[0] = image_data_itr[2]; // blue
        data[1] = image_data_itr[1]; // green
        data[2] = image_data_itr[0]; // red

        data += 4;
        image_data_itr += 4;
        --pixels_to_copy;
    }
}

xcb_image_t*
wpd_create_xcb_image_band(
    xcb_connection_t                *connection,
    const xcb_setup_t               *setup,
    const struct wpd_image_t        *image,
    uint32_t                         row_index,
    uint32_t                         row_count)
{
    assert(setup);
    //assert(geometry);

    xcb_image_t                     *xcb_image;
    uint8_t                         *data;
    //uint8_t                          byte_depth;
    int                              byte_count;
    xcb_format_t                    *format;

    // TODO: Why are all these numbers hard-coded?
    //byte_depth = geometry->depth / 8;
    //assert(byte_depth == 3);
    uint8_t depth = 24;

    uint8_t bits_per_pixel = 32;
    byte_count = image->width * row_count * (bits_per_pixel / 8);
    data = malloc(byte_count);

    format = wpd_find_format(connection, depth, bits_per_pixel);

    wpd_get_xcb_image_band_data(image, row_index, row_count, data);

    xcb_image = xcb_image_create(
        image->width,
        row_count,
        XCB_IMAGE_FORMAT_Z_PIXMAP,
        format->scanline_pad,
        format->depth,
        format->bits_per_pixel,
        0,
        setup->image_byte_order,
        XCB_IMAGE_ORDER_MSB_FIRST,
        data,
        byte_count,
        data);

    if (!xcb_image)
    {
        printf("Failed to create the xcb image\n");
        return NULL;
    }

    return xcb_image;
}

wpd_error_t
wpd_xcb_image_put_chunked(
    xcb_connection_t        *connection,
    const xcb_setup_t       *setup,
    xcb_pixmap_t             pixmap,
    xcb_gcontext_t           gcontext,
    struct wpd_image_t      *image)
{
    xcb_image_t *xcb_image;
    uint32_t     max_request_length_bytes,
                 bytes_per_row,
                 rows_per_request,
                 row_index;

    max_request_length_bytes = xcb_get_maximum_request_length(connection) * 4;

    bytes_per_row = image->width * image->bytes_per_pixel; 

    // TODO: The image data is only one field in the request so I am adding an
    // extra 4KB of padding to be safe. We should come up with a more
    // sophisticated way to calculate the rest of the packet's size and
    // maximize the number of rows we can copy per request.
    rows_per_request = ((max_request_length_bytes - 4096) / bytes_per_row);

    row_index = 0;
    while (row_index < image->height)
    {
        uint32_t rows_remaining = image->height - row_index;
        uint32_t row_count = wpd_min(rows_per_request, rows_remaining);

        // TODO: Create an xcb_image_t for this specific "band"
        xcb_image = wpd_create_xcb_image_band(connection, setup, image,
            row_index, row_count);

        xcb_image_put(connection, pixmap, gcontext, xcb_image, 0, row_index, 0);

        row_index += row_count;
    }

    return WPD_ERROR_SUCCESS;
}

xcb_pixmap_t
wpd_put_image_on_pixmap(
    xcb_connection_t        *connection,
    const xcb_setup_t       *setup,
    const xcb_screen_t      *screen,
    const xcb_window_t       window,
    struct wpd_image_t      *image)
{
    xcb_pixmap_t pixmap;
    xcb_gcontext_t gcontext;
    uint32_t gcontext_mask;
    uint32_t gcontext_values[2];

    // Create a pixmap we will copy the image onto
    pixmap = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 24, pixmap, window, image->width,
        image->height);

    // Create a graphics context to perform the copy
    gcontext_mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    gcontext_values[0] = screen->black_pixel;
    gcontext_values[1] = 0xffffff;

    gcontext = xcb_generate_id(connection);
    xcb_create_gc(connection, gcontext, pixmap, gcontext_mask,
        gcontext_values);

    // Copy the image onto the pixmap, splitting it into horizontal bands as
    // necessary so as not to exceed the maximum xcb request length
    wpd_xcb_image_put_chunked(connection, setup, pixmap, gcontext, image);

    return pixmap;
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

wpd_error_t
wpd_set_wallpaper(
    xcb_connection_t                *connection,
    const xcb_setup_t               *setup,
    const xcb_screen_t              *screen,
    struct xcb_get_geometry_reply_t *geometry,
    const char                      *image_path)
{
    struct wpd_image_t             *image;
    xcb_pixmap_t                    pixmap;
    
    TRY(wpd_get_image(image_path, &image));
    
    pixmap = wpd_put_image_on_pixmap(connection, setup, screen, screen->root,
        image);

    set_pixmap_atom(connection, "_XROOTPMAP_ID", screen->root, pixmap);
    set_pixmap_atom(connection, "ESETROOT_PMAP_ID", screen->root, pixmap);

    xcb_clear_area(connection, 1, screen->root, 0, 0,
        geometry->width, geometry->height);

    xcb_flush(connection);

    wpd_free_image(&image);

    return WPD_ERROR_SUCCESS;
}

wpd_error_t
wpd_set_wallpaper_for_screen(
    const struct wpd_db_t *db,
    xcb_connection_t *connection,
    const xcb_setup_t *setup,
    xcb_screen_t *screen)
{
    xcb_get_geometry_cookie_t    cookie;
    xcb_generic_error_t         *error = NULL;
    xcb_get_geometry_reply_t    *geometry = NULL;
    xcb_window_t                 root_window;
    char                        *image_path = NULL;

    root_window = screen->root;
    LOGINFO("Getting geometry for root window (Id = %u)\n", root_window);

    cookie = xcb_get_geometry(connection, root_window);
    geometry = xcb_get_geometry_reply(connection, cookie, &error);

    if (error)
    {
        print_error(error);
        return WPD_ERROR_TODO;
    }

    print_geometry(geometry);

    LOGINFO("Finding images with compatible dimensions\n");

    // Get compatible images
    TRY(select_random_wallpaper(db, geometry->width, geometry->height,
        &image_path));
    wpd_set_wallpaper(connection, setup, screen, geometry,
        image_path);

    free(geometry);

    return WPD_ERROR_SUCCESS;
}

wpd_error_t
wpd_set_wallpapers(
    const struct wpd_db_t *db)
{
    xcb_connection_t        *connection;
    const xcb_setup_t       *setup;
    xcb_screen_iterator_t    screen_iterator;
    uint8_t                  screen_index;
        
    connection = xcb_connect(NULL, NULL);
    setup = xcb_get_setup(connection);

    screen_index = 1;
    screen_iterator = xcb_setup_roots_iterator(setup);
    while (screen_iterator.rem)
    {
        LOGINFO("Processing screen %u\n", screen_index++);
        wpd_set_wallpaper_for_screen(db, connection, setup, screen_iterator.data);
        xcb_screen_next(&screen_iterator);
    }

    xcb_disconnect(connection);

    return WPD_ERROR_SUCCESS;
}

