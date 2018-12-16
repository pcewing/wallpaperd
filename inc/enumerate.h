#ifndef ENUMERATE_H
#define ENUMERATE_H

#define ftw_error_t int

#define FILE_ENUMERATION_SUCCESS 0
#define FILE_ENUMERATION_ERROR_UNKNOWN_FILENAME 1
#define FILE_ENUMERATION_ERROR_UNKNOWN_EXTENSION 2
#define FILE_ENUMERATION_ERROR_NULL_PARAM 3
#define FILE_ENUMERATION_ERROR_TODO 4

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
ftw_error_t
enumerate_files(const char * path, struct file_enumeration_t** result);

ftw_error_t
iterate_file_enumeration(const struct file_enumeration_t* enumeration,
    void (*f) (const struct file_enumeration_node_t* node));

ftw_error_t
free_enumeration(struct file_enumeration_t** enumeration);

#endif

