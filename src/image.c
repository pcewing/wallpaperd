#include "image.h"
#include "log.h"
#include "stb_image.h"

#include <stdlib.h>

#define BYTES_PER_PIXEL 4

//------------------------------------------------------------------------------
wpd_error_t
wpd_get_image(const char* path, struct wpd_image_t** result)
{
    int x, y, n;
    unsigned char *data;
    struct wpd_image_t* image;
    
    data = stbi_load(path, &x, &y, &n, BYTES_PER_PIXEL);
    if (!data)
    {
        LOGERROR("STBI_LOAD failure");
        return WPD_ERROR_IMAGE_STBI_LOAD_FAILURE;
    }

    image = malloc(sizeof(struct wpd_image_t));
    image->width = x;
    image->height = y;
    image->comp = n;
    image->bytes_per_pixel = BYTES_PER_PIXEL;
    image->data = data;
    
    *result = image;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

//------------------------------------------------------------------------------
wpd_error_t
wpd_free_image(struct wpd_image_t** image)
{
    if (!image || !(*image))
    {
        return WPD_ERROR_GLOBAL_NULL_PARAM;
    }

    if ((*image)->data)
    {
        stbi_image_free((*image)->data);
    }
    
    free(*image);
    image = NULL;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

//------------------------------------------------------------------------------
wpd_error_t
wpd_get_image_metadata(
    const char *path,
    struct wpd_image_metadata_t **result)
{
    int width, height, comp;
    struct wpd_image_metadata_t *image_metadata;

    stbi_info(path, &width, &height, &comp);

    image_metadata = malloc(sizeof(struct wpd_image_metadata_t));
    image_metadata->width = width;
    image_metadata->height = height;

    *result = image_metadata;
    return WPD_ERROR_GLOBAL_SUCCESS;
}

//------------------------------------------------------------------------------
wpd_error_t
wpd_free_image_metadata(struct wpd_image_metadata_t** image_metadata)
{
    if (!image_metadata || !(*image_metadata))
    {
        return WPD_ERROR_GLOBAL_NULL_PARAM;
    }

    free(*image_metadata);
    image_metadata = NULL;

    return WPD_ERROR_GLOBAL_SUCCESS;
}

