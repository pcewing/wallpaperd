#ifndef FTW_H
#define FTW_H

#include "core.h"
#include "data.h"

#define MAX_NODES 16384

/**
 * Recursively traverses the path provided and inserts any image files found
 * into the database. Environment variables in the path will are expanded.
 **/
wpd_error_t wpd_ftw(const struct wpd_db_t *db, const char *path);

#endif // FTW_H
