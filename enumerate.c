#define _XOPEN_SOURCE 600       /* Get nftw() and S_IFSOCK declarations */

#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <stdbool.h>

#include "enumerate.h"
#include "util.h"

const char * file_types[] = { "jpg", "png" };

/* State tracking vars */
struct file_enumeration_t* current_result;

char* 
ftw_type_str(const int type)
{
    char * result;
    switch(type)
    {
        case FTW_F:   result = "FTW_F";   break;
        case FTW_D:   result = "FTW_D";   break;
        case FTW_DNR: result = "FTW_DNR"; break;
        case FTW_DP:  result = "FTW_DP";  break;
        case FTW_SL:  result = "FTW_SL";  break;
        case FTW_SLN: result = "FTW_SLN"; break;
        case FTW_NS:  result = "FTW_NS";  break;
        default: result = "UNDEFINED";
    };
    return result;
}

char*
st_mode_str(const int st_mode)
{
    char * result;
    switch (st_mode)
    {
        case S_IFREG:  result = "S_IFREG";  break;
        case S_IFDIR:  result = "S_IFDIR";  break;
        case S_IFCHR:  result = "S_IFCHR";  break;
        case S_IFBLK:  result = "S_IFBLK";  break;
        case S_IFLNK:  result = "S_IFLNK";  break;
        case S_IFIFO:  result = "S_IFIFO";  break;
        case S_IFSOCK: result = "S_IFSOCK"; break;
        default: result = "UNDEFINED";
    };
    return result;
}

ftw_error_t
create_file_enumeration_node(const char * path, struct file_enumeration_node_t** out)
{
    (*out) = malloc(sizeof(struct file_enumeration_node_t));
    (*out)->m_path = strdup(path);

    char *filename = getfilename(path);
    if (!filename)
    {
        return FILE_ENUMERATION_ERROR_UNKNOWN_FILENAME;
    }

    (*out)->m_file = strdup(filename);

    char *extension = getextension(filename);
    if (!extension)
    {
        return FILE_ENUMERATION_ERROR_UNKNOWN_EXTENSION;
    }

    (*out)->m_ext = strdup(extension);

    return FILE_ENUMERATION_SUCCESS;
}

ftw_error_t
add_file_enumeration_node(struct file_enumeration_node_t* node)
{
    if (!node)
    {
        return FILE_ENUMERATION_ERROR_NULL_PARAM;
    }

    current_result->nodes[current_result->node_count] = node;
    ++current_result->node_count;

    return FILE_ENUMERATION_SUCCESS;
}

ftw_error_t
iterate_file_enumeration(const struct file_enumeration_t* enumeration,
    void (*f) (const struct file_enumeration_node_t* node))
{
    if (!f)
    {
        return FILE_ENUMERATION_ERROR_NULL_PARAM;
    }

    for (size_t i = 0; i < enumeration->node_count; ++i)
    {
        f(enumeration->nodes[i]);
    }

    return FILE_ENUMERATION_SUCCESS;
}

ftw_error_t
free_file_enumeration_node(struct file_enumeration_node_t** node)
{
    if (!node)
    {
        return FILE_ENUMERATION_ERROR_NULL_PARAM;
    }

    if ((*node)->m_path) free((void*)(*node)->m_path);
    if ((*node)->m_file) free((void*)(*node)->m_file);
    if ((*node)->m_ext) free((void*)(*node)->m_ext);

    free(*node);
    node = NULL;

    return FILE_ENUMERATION_SUCCESS;
}

ftw_error_t
free_enumeration(struct file_enumeration_t** enumeration)
{
    if (!enumeration)
    {
        return FILE_ENUMERATION_ERROR_NULL_PARAM;
    }

    for (size_t i = 0; i < (*enumeration)->node_count; ++i)
    {
        free_file_enumeration_node(&(*enumeration)->nodes[i]);
    }

    free((void*)(*enumeration));
    enumeration = NULL;

    return FILE_ENUMERATION_SUCCESS;
}

void
process_node(const char * path)
{
    struct file_enumeration_node_t* node = NULL;
    if (create_file_enumeration_node(path, &node) != FILE_ENUMERATION_SUCCESS)
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

ftw_error_t
enumerate_files(const char * path, struct file_enumeration_t** result)
{
    current_result = malloc(sizeof(struct file_enumeration_t));

    if (nftw(path, nftw_handler, 10, 0) == -1)
    {
        /* TODO: Free the result */
        return FILE_ENUMERATION_ERROR_TODO;
    }

    (*result) = current_result;
    current_result = NULL;

    return FILE_ENUMERATION_SUCCESS;
}

