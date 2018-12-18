#define _XOPEN_SOURCE 600       /* Get nftw() and S_IFSOCK declarations */

#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <stdbool.h>

#include "enumerate.h"
#include "core.h"

const char * file_types[] = { "jpg", "png" };

/* State tracking vars */
struct file_enumeration_t* current_result;

wpd_error
create_file_enumeration_node(const char * path, struct file_enumeration_node_t** out)
{
    (*out) = malloc(sizeof(struct file_enumeration_node_t));
    (*out)->m_path = strdup(path);

    char *filename = wpd_get_filename(path);
    if (!filename)
    {
        return WPD_ERROR_UNKNOWN_FILENAME;
    }

    (*out)->m_file = strdup(filename);

    char *extension = wpd_get_extension(filename);
    if (!extension)
    {
        return WPD_ERROR_UNKNOWN_EXTENSION;
    }

    (*out)->m_ext = strdup(extension);

    return WPD_ERROR_SUCCESS;
}

wpd_error
add_file_enumeration_node(struct file_enumeration_node_t* node)
{
    if (!node)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    current_result->nodes[current_result->node_count] = node;
    ++current_result->node_count;

    return WPD_ERROR_SUCCESS;
}

wpd_error
iterate_file_enumeration(const struct file_enumeration_t* enumeration,
    void (*f) (const struct file_enumeration_node_t* node))
{
    if (!f)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    for (size_t i = 0; i < enumeration->node_count; ++i)
    {
        f(enumeration->nodes[i]);
    }

    return WPD_ERROR_SUCCESS;
}

wpd_error
free_file_enumeration_node(struct file_enumeration_node_t** node)
{
    if (!node)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    if ((*node)->m_path) free((void*)(*node)->m_path);
    if ((*node)->m_file) free((void*)(*node)->m_file);
    if ((*node)->m_ext) free((void*)(*node)->m_ext);

    free(*node);
    node = NULL;

    return WPD_ERROR_SUCCESS;
}

wpd_error
free_enumeration(struct file_enumeration_t** enumeration)
{
    if (!enumeration)
    {
        return WPD_ERROR_NULL_PARAM;
    }

    for (size_t i = 0; i < (*enumeration)->node_count; ++i)
    {
        free_file_enumeration_node(&(*enumeration)->nodes[i]);
    }

    free((void*)(*enumeration));
    enumeration = NULL;

    return WPD_ERROR_SUCCESS;
}

void
process_node(const char * path)
{
    struct file_enumeration_node_t* node = NULL;
    if (create_file_enumeration_node(path, &node) != WPD_ERROR_SUCCESS)
    {
        return;
    }

    bool supported_file_type = false;
    for (unsigned int i = 0; i < sizeof(file_types) / sizeof(char*); ++i)
    {
        if (strcmp(node->m_ext, file_types[i]) == 0)
        {
            supported_file_type = true;
            break;
        }
    }

    if (supported_file_type)
    {
        add_file_enumeration_node(node);
    }
}


static int
nftw_handler(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
    UNUSED(ftwb);

    if ((sbuf->st_mode & S_IFMT) != S_IFREG
        || type != FTW_F)
    {
        return 0;
    }

    process_node(pathname);

    return 0;
}

wpd_error
enumerate_files(const char * path, struct file_enumeration_t** result)
{
    current_result = malloc(sizeof(struct file_enumeration_t));

    if (nftw(path, nftw_handler, 10, 0) == -1)
    {
        /* TODO: Free the result */
        return WPD_ERROR_TODO;
    }

    (*result) = current_result;
    current_result = NULL;

    return WPD_ERROR_SUCCESS;
}

