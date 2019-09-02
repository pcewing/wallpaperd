#ifndef CORE_H
#define CORE_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "error.g.h"

#define UNUSED(x) (void)(x)

/**
 * Terminates the application with the specified error code.
 **/
void
wpd_exit(wpd_error_t exit_code);

/**
 * Strips directories from a path leaving just the file name.
 *
 * Note: Returns a pointer that indicates the position in pathname where the
 * file name starts; this should not be freed.
 **/
char*
wpd_get_filename(const char *pathname);

/**
 * Strips directories and file name from a path leaving just the extension.
 *
 * Note: Returns a pointer that indicates the position in pathname where the
 * file name starts; this should not be freed.
 **/
char*
wpd_get_extension(const char *path);

/**
 * Sleeps for a specified number of seconds.
 **/
unsigned int
wpd_sleep(unsigned int seconds);

/**
 * Seeds the global random number generator.
 **/
void
wpd_srand();

/**
 * Retrieves the next value from the global random number generator.
 **/
int
wpd_rand();

/**
 * Gets the minimum value between two numbers.
 **/
uint32_t
wpd_min(uint32_t a, uint32_t b);

/**
 * Joins two paths.
 *
 * Note: The result is placed in a newly allocated string and must be manually
 * freed by the caller.
 **/
char*
wpd_path_join(const char *a, const char *b);

/**
 * Converts a string to lower-case.
 *
 * Note: The result is placed in a newly allocated string and must be manually
 * freed by the caller.
 **/
char*
wpd_strdup_lower(const char *data);

#endif // CORE_H

