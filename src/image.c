#include "image.h"
#include "log.h"
#include "stb_image.h"

#include <stdlib.h>

#define PREFERRED_BYTE_DEPTH 4

//------------------------------------------------------------------------------
wpd_error_t
wpd_create_image(const char* path, struct wpd_image_t** result)
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

//------------------------------------------------------------------------------
wpd_error_t
wpd_destroy_image(struct wpd_image_t** image)
{
    if (!image || !(*image) || !(*image)->data)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    stbi_image_free((*image)->data);
    free(*image);

    return WPD_ERROR_SUCCESS;
}


//------------------------------------------------------------------------------
struct wpd_image_metadata_t*
wpd_create_image_metadata(const char *filename)
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

//------------------------------------------------------------------------------
wpd_error_t
wpd_destroy_image_metadata(struct wpd_image_metadata_t** image_metadata)
{
    if (!image_metadata || !(*image_metadata))
    {
        return WPD_ERROR_NULL_PARAM;
    }

    FREE((*image_metadata)->path);
    FREE(*image_metadata);

    return WPD_ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
wpd_error_t
wpd_create_image_metadata_array(
    const struct wpd_file_enumeration_t *enumeration,
    struct wpd_image_metadata_array_t** out)
{
    struct wpd_image_metadata_array_t *image_metadata_array;
    int i;

    if (!enumeration)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    image_metadata_array = malloc(sizeof(struct wpd_image_metadata_array_t));
    image_metadata_array->count = enumeration->node_count;
    image_metadata_array->data =
        malloc(enumeration->node_count * sizeof(struct wpd_image_metadata_t*));

    for (i = 0; i < enumeration->node_count; ++i)
    {
        image_metadata_array->data[i] =
            wpd_create_image_metadata(enumeration->nodes[i]->path);
    }

    (*out) = image_metadata_array;
    return WPD_ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
wpd_error_t
wpd_destroy_image_metadata_array(
    struct wpd_image_metadata_array_t **image_metadata_array)
{
    int i;

    if (!image_metadata_array || !(*image_metadata_array))
    {
        return WPD_ERROR_NULL_PARAM;
    }

    for (i = 0; i < (*image_metadata_array)->count; ++i)
    {
        wpd_destroy_image_metadata(&((*image_metadata_array)->data[i]));
    }

    FREE((*image_metadata_array)->data);
    FREE(*image_metadata_array);

    return WPD_ERROR_SUCCESS;
}

