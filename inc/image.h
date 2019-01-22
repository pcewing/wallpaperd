#ifndef IMAGE_H
#define IMAGE_H

#include "core.h"

struct wpd_image_metadata_t
{
    int width;
    int height;
};

struct wpd_image_t
{
    int width;
    int height;
    int comp;
    int bytes_per_pixel;
    unsigned char *data;
};

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_get_image(
    const char* path,
    struct wpd_image_t** result);

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_free_image(
    struct wpd_image_t** image);

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_get_image_metadata(
    const char *filename,
    struct wpd_image_metadata_t **result);

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_free_image_metadata(
    struct wpd_image_metadata_t** image_metadata);

#endif // IMAGE_H
