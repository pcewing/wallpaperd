#ifndef ENUMERATE2_H
#define ENUMERATE2_H

#include "core.h"
#include "data.h"

#define MAX_NODES 16384

struct wpd_file_enumeration_node_t
{
    char* path;
    char* file;
    char* ext;
};

struct wpd_file_enumeration_t
{
    struct wpd_file_enumeration_node_t* nodes[MAX_NODES];
    size_t node_count;
};

/**
 * TODO: Document this.
 **/
wpd_error_t
wpd_ftw(
    const struct wpd_db_t* db,
    const char *path);

/**
 * TODO: Document this.
 **/
//wpd_error_t
//wpd_create_file_enumeration(
//    const char                     *path,
//    struct wpd_file_enumeration_t **result);

/**
 * TODO: Document this.
 **/
//wpd_error_t
//wpd_destroy_file_enumeration(
//    struct wpd_file_enumeration_t** enumeration);


#endif // ENUMERATE2_H

