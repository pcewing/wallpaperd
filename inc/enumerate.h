#ifndef ENUMERATE_H
#define ENUMERATE_H

#include "core.h"

#define MAX_NODES 1024

struct file_enumeration_node_t
{
    char* path;
    char* file;
    char* ext;
};

struct file_enumeration_t
{
    struct file_enumeration_node_t* nodes[MAX_NODES];
    size_t node_count;
};

/**
 * TODO: Document this.
 **/
wpd_error_t
create_file_enumeration(
    const char                 *path,
    struct file_enumeration_t **result);

/**
 * TODO: Document this.
 **/
wpd_error_t
destroy_file_enumeration(
    struct file_enumeration_t** enumeration);

#endif // ENUMERATE_H

