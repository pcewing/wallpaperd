#ifndef CORE_H
#define CORE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "error.g.h"

#define UNUSED(x) (void)(x)

#define ARRLEN(x) (sizeof(x) / sizeof((x)[0]))
#define ARRLEN_U32(x) (uint32_t) ARRLEN(x)

/**
 * Terminates the application with the specified error code.
 **/
void wpd_exit(wpd_error_t exit_code);

/**
 * Strips directories from a path leaving just the file name.
 *
 * Note: Returns a pointer that indicates the position in pathname where the
 * file name starts; this should not be freed.
 **/
char *wpd_get_filename(const char *pathname);

/**
 * Strips directories and file name from a path leaving just the extension.
 *
 * Note: Returns a pointer that indicates the position in pathname where the
 * file name starts; this should not be freed.
 **/
char *wpd_get_extension(const char *path);

/**
 * Sleeps for a specified number of seconds.
 **/
unsigned int wpd_sleep(unsigned int seconds);

/**
 * Sleeps for a specified number of microseconds.
 **/
unsigned int wpd_usleep(unsigned int seconds);

/**
 * Gets the current system time in microseconds.
 **/
uint64_t get_timestamp_us();

/**
 * Seeds the global random number generator.
 **/
void wpd_srand();

/**
 * Retrieves the next value from the global random number generator.
 **/
int wpd_rand();

/**
 * Gets the minimum value between two numbers.
 **/
uint32_t wpd_min(uint32_t a, uint32_t b);

/**
 * Joins two paths.
 *
 * Note: The result is placed in a newly allocated string and must be manually
 * freed by the caller.
 **/
char *wpd_path_join(const char *a, const char *b);

/**
 * Converts a string to lower-case.
 *
 * Note: The result is placed in a newly allocated string and must be manually
 * freed by the caller.
 **/
char *wpd_strdup_lower(const char *data);

/**
 * Checks whether or not the needle matches any strings in the haystack.
 **/
bool strmatch(const char **haystack, const char *needle);

/**
 * Gets the string description for an OS error code.
 **/
char *wpd_strerror(int errnum);

#endif // CORE_H
