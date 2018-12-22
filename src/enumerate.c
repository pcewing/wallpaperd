#define _XOPEN_SOURCE 600       /* Get nftw() and S_IFSOCK declarations */

#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <stdbool.h>

#include "log.h"
#include "enumerate.h"
#include "core.h"

// TODO: Make this configurable
const char * file_types[] = { "jpg", "png" };

// TODO: Don't use ftw which forces the usage of a global variable like this
struct wpd_file_enumeration_t* current_result;

wpd_error_t
wpd_create_file_enumeration_node(const char * path, struct wpd_file_enumeration_node_t** out)
{
    struct wpd_file_enumeration_node_t  *result;
    char                            *filename;
    char                            *extension;

    result = malloc(sizeof(struct wpd_file_enumeration_node_t));
    result->path = strdup(path);

    filename = wpd_get_filename(path);
    if (!filename)
    {
        return WPD_ERROR_UNKNOWN_FILENAME;
    }

    result->file = strdup(filename);

    extension = wpd_get_extension(filename);
    if (!extension)
    {
        return WPD_ERROR_UNKNOWN_EXTENSION;
    }

    result->ext = strdup(extension);

    (*out) = result;
    return WPD_ERROR_SUCCESS;
}

wpd_error_t
wpd_add_file_enumeration_node(struct wpd_file_enumeration_node_t* node)
{
    if (!node)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    current_result->nodes[current_result->node_count] = node;
    ++current_result->node_count;

    return WPD_ERROR_SUCCESS;
}

wpd_error_t
wpd_destroy_file_enumeration_node(struct wpd_file_enumeration_node_t** node)
{
    if (!node || !(*node))
    {
        return WPD_ERROR_NULL_PARAM;
    }

    FREE((*node)->path);
    FREE((*node)->file);
    FREE((*node)->ext);
    FREE(*node);

    return WPD_ERROR_SUCCESS;
}

wpd_error_t
wpd_destroy_file_enumeration(struct wpd_file_enumeration_t** enumeration)
{
    if (!enumeration)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    for (size_t i = 0; i < (*enumeration)->node_count; ++i)
    {
        TRY(wpd_destroy_file_enumeration_node(&(*enumeration)->nodes[i]));
    }

    FREE(*enumeration);

    return WPD_ERROR_SUCCESS;
}

void
wpd_process_node(const char * path)
{
    struct wpd_file_enumeration_node_t* node = NULL;
    if (wpd_create_file_enumeration_node(path, &node) != WPD_ERROR_SUCCESS)
    {
        return;
    }

    bool supported_file_type = false;
    for (unsigned int i = 0; i < sizeof(file_types) / sizeof(char*); ++i)
    {
        if (strcmp(node->ext, file_types[i]) == 0)
        {
            supported_file_type = true;
            break;
        }
    }

    if (supported_file_type)
    {
        wpd_add_file_enumeration_node(node);
    }
}


static int
wpd_nftw_handler(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
    UNUSED(ftwb);

    if ((sbuf->st_mode & S_IFMT) != S_IFREG
        || type != FTW_F)
    {
        return 0;
    }

    wpd_process_node(pathname);

    return 0;
}

wpd_error_t
wpd_create_file_enumeration(const char * path, struct wpd_file_enumeration_t** result)
{
    current_result = malloc(sizeof(struct wpd_file_enumeration_t));

    if (nftw(path, wpd_nftw_handler, 10, 0) == -1)
    {
        /* TODO: Free the result */
        return WPD_ERROR_TODO;
    }

    LOGINFO("Found %i wallpaper files!", current_result->node_count);

    (*result) = current_result;
    current_result = NULL;

    return WPD_ERROR_SUCCESS;
}

