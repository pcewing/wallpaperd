#ifndef IMAGE_H
#define IMAGE_H

#include "core.h"

/**
 * Represents the metadata associated with an image.
 **/
struct wpd_image_metadata_t {
    unsigned int width;
    unsigned int height;
};

/**
 * Represents an image.
 **/
struct wpd_image_t {
    unsigned int width;
    unsigned int height;
    unsigned int comp;
    unsigned int bytes_per_pixel;
    unsigned char *data;
};

/**
 * Reads image data from a specified path.
 *
 * Note: The resulting image needs to be manually destroyed by the caller via
 * wpd_free_image().
 **/
wpd_error_t wpd_get_image(const char *path, struct wpd_image_t **result);

/**
 * Frees all resources associated with an image object.
 **/
wpd_error_t wpd_free_image(struct wpd_image_t **image);

/**
 * Reads image metadata from a specified path.
 *
 * Note: The resulting image needs to be manually destroyed by the caller via
 * wpd_free_image_metadata().
 **/
wpd_error_t wpd_get_image_metadata(const char *filename,
                                   struct wpd_image_metadata_t **result);

/**
 * Frees all resources associated with an image metadata object.
 **/
wpd_error_t
wpd_free_image_metadata(struct wpd_image_metadata_t **image_metadata);

#endif // IMAGE_H
