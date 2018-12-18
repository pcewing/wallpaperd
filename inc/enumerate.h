#ifndef ENUMERATE_H
#define ENUMERATE_H

#include "core.h"

#define MAX_NODES 1024

struct file_enumeration_node_t
{
    char* m_path;
    char* m_file;
    char* m_ext;
};

struct file_enumeration_t
{
    struct file_enumeration_node_t* nodes[MAX_NODES];
    size_t node_count;
};

// Public interface

//
wpd_error
enumerate_files(const char * path, struct file_enumeration_t** result);

wpd_error
iterate_file_enumeration(const struct file_enumeration_t* enumeration,
    void (*f) (const struct file_enumeration_node_t* node));

wpd_error
free_enumeration(struct file_enumeration_t** enumeration);

#endif

