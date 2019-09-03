#include "wallpaper.h"
#include "log.h"
#include "image.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

char *
xcb_generic_error_to_json(const xcb_generic_error_t *error)
{
    const char * fmt = "{\n"
        "  \"response_type\": %u,\n"
        "  \"error_code\": %u,\n"
        "  \"sequence\": %u,\n"
        "  \"resource_id\": %u,\n"
        "  \"minor_code\": %u,\n"
        "  \"major_code\": %u,\n"
        "  \"full_sequence\": %u,\n"
        "}\n";

    int length = 1 + snprintf(NULL, 0, fmt, error->response_type,
            error->error_code, error->sequence, error->resource_id,
            error->minor_code, error->major_code, error->full_sequence);

    char * result = malloc(length * sizeof(char));
    if (result)
    {
        snprintf(result, length, fmt, error->response_type, error->error_code,
                error->sequence, error->resource_id, error->minor_code,
                error->major_code, error->full_sequence);
    }

    return result;
}

char *
xcb_get_geometry_reply_to_json(const xcb_get_geometry_reply_t *geometry)
{
    const char * fmt = "{"
            " \"response_type\": %u,"
            " \"depth\": %u,"
            " \"sequence\": %u,"
            " \"length\": %u,"
            " \"root\": %u,"
            " \"x\": %u,"
            " \"y\": %u,"
            " \"width\": %u,"
            " \"height\": %u,"
            " \"border_width\": %u }";

    int length = 1 + snprintf(NULL, 0, fmt, geometry->response_type,
            geometry->depth, geometry->sequence, geometry->length,
            geometry->root, geometry->x, geometry->y, geometry->width,
            geometry->height, geometry->border_width);

    char * result = malloc(length * sizeof(char));
    if (result)
    {
        snprintf(result, length, fmt, geometry->response_type,
                geometry->depth, geometry->sequence, geometry->length,
                geometry->root, geometry->x, geometry->y, geometry->width,
                geometry->height, geometry->border_width);
    }

    return result;
}


void
print_error(const xcb_generic_error_t *error)
{
    assert(error);

    char * json = xcb_generic_error_to_json(error);
    if (json)
    {
        LOGINFO("Error: %s", json);
        free(json);
    }
}

void
print_geometry(const xcb_get_geometry_reply_t *geometry)
{
    assert(geometry);

    char * json = xcb_get_geometry_reply_to_json(geometry);
    if (json)
    {
        LOGINFO("Geometry: %s", json);
        free(json);
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

    index = wpd_rand() % wallpapers->count;
    image_path = strdup(wallpapers->results[index].path);

    for (i = 0; i < wallpapers->count; ++i)
    {
        free(wallpapers->results[i].path);
    }
    free(wallpapers->results);
    free(wallpapers);

    *result = image_path;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

/*
 * TODO: Grok this; it's taken from:
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
    return NULL;
}

void
wpd_get_xcb_image_band_data(const struct wpd_image_t *image, int row_index,
        int row_count, unsigned char *data)
{
    int pixels_to_copy = image->width * row_count;

    int offset = row_index * image->width * image->bytes_per_pixel;
    unsigned char *image_data_itr = image->data + offset;
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
wpd_create_xcb_image_band(xcb_connection_t *connection,
        const xcb_setup_t *setup, const struct wpd_image_t *image,
        uint32_t row_index, uint32_t row_count)
{
    const uint8_t depth = 24;
    const uint8_t bits_per_pixel = 32;
    int byte_count = image->width * row_count * (bits_per_pixel / 8);

    // Note that we don't need to manually free this; it will be freed when we
    // call xcb_image_destroy on the image we are creating.
    uint8_t *data = malloc(byte_count);

    xcb_format_t *format = wpd_find_format(connection, depth, bits_per_pixel);
    assert(format);

    wpd_get_xcb_image_band_data(image, row_index, row_count, data);

    xcb_image_t *xcb_image = xcb_image_create(image->width, row_count,
            XCB_IMAGE_FORMAT_Z_PIXMAP, format->scanline_pad, format->depth,
            format->bits_per_pixel, 0, setup->image_byte_order,
            XCB_IMAGE_ORDER_MSB_FIRST, data, byte_count, data);

    if (!xcb_image)
    {
        LOGERROR("Failed to create the xcb image");
        free(data);
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

        // Create an xcb_image_t for this specific "band"
        xcb_image_t *xcb_image = wpd_create_xcb_image_band(connection, setup, image,
            row_index, row_count);

        xcb_image_put(connection, pixmap, gcontext, xcb_image, 0, row_index, 0);

        xcb_image_destroy(xcb_image);

        row_index += row_count;
    }

    return WPD_ERROR_GLOBAL_SUCCESS;
}

xcb_pixmap_t
wpd_put_image_on_pixmap(
    xcb_connection_t        *connection,
    const xcb_setup_t       *setup,
    const xcb_screen_t      *screen,
    const xcb_window_t       window,
    struct wpd_image_t      *image)
{
    // Create a pixmap we will copy the image onto
    xcb_pixmap_t pixmap = xcb_generate_id(connection);
    xcb_create_pixmap(connection, 24, pixmap, window, image->width,
        image->height);

    // Create a graphics context to perform the copy
    uint32_t gcontext_mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
    uint32_t gcontext_values[] = { screen->black_pixel, 0xffffff };

    xcb_gcontext_t gc = xcb_generate_id(connection);
    xcb_create_gc(connection, gc, pixmap, gcontext_mask, gcontext_values);

    // Copy the image onto the pixmap, splitting it into horizontal bands as
    // necessary so as not to exceed the maximum xcb request length
    wpd_xcb_image_put_chunked(connection, setup, pixmap, gc, image);

    xcb_void_cookie_t xcb_free_gc_cookie = xcb_free_gc(connection, gc);
    xcb_generic_error_t *xcb_free_gc_error = xcb_request_check(connection,
            xcb_free_gc_cookie);
    assert(!xcb_free_gc_error);

    return pixmap;
}

// Note: The structure returned by this method needs to be freed by the caller
xcb_atom_t
get_atom(xcb_connection_t *connection, const char *atom_name)
{
    static const bool only_if_exists = true;

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection,
            only_if_exists, strlen(atom_name), atom_name);

    xcb_generic_error_t *error;
    xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie,
            &error);

    // TODO: Do we really need to assert here?
    assert(!error);
    assert(reply);

    xcb_atom_t atom = reply->atom;

    free(reply);

    return atom;
}

// Note: The structure returned by this method needs to be freed by the caller
xcb_get_property_reply_t *
get_pixmap_property(
    xcb_connection_t        *connection,
    xcb_window_t             window,
    xcb_atom_t               atom)
{
    static const uint8_t delete = 0;
    static const uint8_t offset = 0;
    static const uint8_t length = 0;

    xcb_get_property_cookie_t get_property_cookie = xcb_get_property(
            connection, delete, window, atom, XCB_ATOM_PIXMAP,
            offset, length);

    xcb_generic_error_t *error;
    xcb_get_property_reply_t *get_property_reply = xcb_get_property_reply(
            connection, get_property_cookie, &error);

    assert(!error);
    assert(get_property_reply);

    return get_property_reply;
}

xcb_pixmap_t
get_pixmap_atom(
    xcb_connection_t *c,
    const char       *atom_name,
    xcb_window_t      window)
{
    xcb_atom_t atom = get_atom(c, atom_name);

    xcb_get_property_reply_t *pixmap_property = get_pixmap_property(c, window,
            atom);

    xcb_pixmap_t pixmap_id = *((xcb_pixmap_t*) xcb_get_property_value(
                pixmap_property));

    free(pixmap_property);

    return pixmap_id;
}

void
set_pixmap_atom(
    xcb_connection_t *c,
    const char *atom_name,
    xcb_window_t window,
    xcb_pixmap_t pixmap_id)
{
    xcb_atom_t atom = get_atom(c, atom_name);

    xcb_get_property_reply_t *pixmap_property = get_pixmap_property(c, window,
            atom);

    xcb_void_cookie_t change_property_cookie;
    change_property_cookie = xcb_change_property_checked(
        c,
        XCB_PROP_MODE_REPLACE,
        window,
        atom,
        pixmap_property->type,
        pixmap_property->format,
        1,
        (unsigned char *)&pixmap_id);

    xcb_generic_error_t *error;
    error = xcb_request_check(c, change_property_cookie);
    assert(!error);

    free(pixmap_property);
}

wpd_error_t
wpd_set_wallpaper(
    xcb_connection_t                *connection,
    const xcb_setup_t               *setup,
    const xcb_screen_t              *screen,
    struct xcb_get_geometry_reply_t *geometry,
    const char                      *image_path)
{
    struct wpd_image_t  *image;
    xcb_pixmap_t         new_pixmap,
                         xroot_pixmap,
                         esetroot_pixmap;
    xcb_generic_error_t *error;
    
    wpd_error_t wpd_error = wpd_get_image(image_path, &image);
    if (wpd_error != WPD_ERROR_GLOBAL_SUCCESS)
    {
        return wpd_error;
    }

    LOGINFO("Setting wallpaper to %s", image_path);

    // Get the existing background pixmap(s)
    xroot_pixmap = get_pixmap_atom(connection, "_XROOTPMAP_ID", screen->root);
    esetroot_pixmap = get_pixmap_atom(connection, "ESETROOT_PMAP_ID",
            screen->root);
    
    // Create a new background pixmap
    new_pixmap = wpd_put_image_on_pixmap(connection, setup, screen,
            screen->root, image);

    // Set the new background pixmap
    set_pixmap_atom(connection, "_XROOTPMAP_ID", screen->root, new_pixmap);
    set_pixmap_atom(connection, "ESETROOT_PMAP_ID", screen->root, new_pixmap);
    
    // Free the old background pixmap(s)
    xcb_void_cookie_t xcb_free_pixmap_cookie = xcb_free_pixmap(connection,
            xroot_pixmap);
    error = xcb_request_check(connection, xcb_free_pixmap_cookie);
    assert(!error);

    if (esetroot_pixmap != xroot_pixmap)
    {
        xcb_free_pixmap_cookie = xcb_free_pixmap(connection, esetroot_pixmap);
        error = xcb_request_check(connection, xcb_free_pixmap_cookie);
        assert(!error);
    }

    xcb_clear_area(connection, 1, screen->root, 0, 0,
        geometry->width, geometry->height);

    xcb_flush(connection);

    wpd_free_image(&image);

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t
wpd_set_wallpaper_for_screen(
    const struct wpd_db_t *db,
    xcb_connection_t *connection,
    const xcb_setup_t *setup,
    xcb_screen_t *screen)
{
    xcb_window_t root_window = screen->root;
    LOGINFO("Getting geometry for root window (Id = %u)", root_window);

    xcb_get_geometry_cookie_t cookie = xcb_get_geometry(connection,
            root_window);

    xcb_generic_error_t *error = NULL;
    xcb_get_geometry_reply_t *geometry = xcb_get_geometry_reply(connection,
            cookie, &error);

    if (error)
    {
        print_error(error);
        return WPD_ERROR_XCB_GET_GEOMETRY_FAILED;
    }

    print_geometry(geometry);

    LOGINFO("Finding images with compatible dimensions");

    // Get compatible images
    char *image_path = NULL;
    wpd_error_t wpd_error = select_random_wallpaper(db, geometry->width,
            geometry->height, &image_path);
    if (wpd_error != WPD_ERROR_GLOBAL_SUCCESS)
    {
        return wpd_error;
    }

    wpd_set_wallpaper(connection, setup, screen, geometry,
        image_path);

    free(geometry);
    free(image_path);

    return WPD_ERROR_GLOBAL_SUCCESS;
}

wpd_error_t
wpd_set_wallpapers(
    const struct wpd_db_t *db)
{
    xcb_connection_t *connection = xcb_connect(NULL, NULL);
    const xcb_setup_t *setup = xcb_get_setup(connection);

    uint8_t screen_index = 1;
    xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(setup);
    while (screen_iterator.rem)
    {
        LOGINFO("Processing screen %u", screen_index++);
        wpd_set_wallpaper_for_screen(db, connection, setup,
                screen_iterator.data);
        xcb_screen_next(&screen_iterator);
    }

    xcb_disconnect(connection);

    return WPD_ERROR_GLOBAL_SUCCESS;
}

