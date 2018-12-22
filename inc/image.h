#ifndef IMAGE_H
#define IMAGE_H

#include "core.h"
#include "enumerate.h"

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

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_create_image(
    const char* path,
    struct wpd_image_t** result);

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_destroy_image(
    struct wpd_image_t** image);

/**
 * TODO: Documentation
 **/
struct wpd_image_metadata_t*
wpd_create_image_metadata(
    const char *filename);

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_destroy_image_metadata(
    struct wpd_image_metadata_t** image_metadata);

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_create_image_metadata_array(
    const struct wpd_file_enumeration_t *enumeration,
    struct wpd_image_metadata_array_t** out);

/**
 * TODO: Documentation
 **/
wpd_error_t
wpd_destroy_image_metadata_array(
    struct wpd_image_metadata_array_t **image_metadata_array);

#endif // IMAGE_H
