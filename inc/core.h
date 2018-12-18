#ifndef UTIL_H
#define UTIL_H

#include <string.h>

#define UNUSED(x) (void)(x)

typedef int wpd_error;

#define WPD_ERROR_SUCCESS 0x0000
#define WPD_ERROR_FAILURE 0x0001

#define WPD_ERROR_UNKNOWN_FILENAME 0x0002
#define WPD_ERROR_UNKNOWN_EXTENSION 0x0003
#define WPD_ERROR_NULL_PARAM 0x0004
#define WPD_ERROR_TODO 0x0005

#define TRY(statement) { \
        wpd_error wpd__error; \
        wpd__error = statement; \
        if (wpd__error != WPD_ERROR_SUCCESS) \
        { \
            return wpd__error; \
        } \
    }

void
wpd_exit(int exit_code);

void
wpd_die(const char * message);

const char*
wpd_error_str(wpd_error error);

char*
wpd_get_filename(const char *pathname);

char*
wpd_get_extension(const char *filename);

unsigned int
wpd_sleep(unsigned int seconds);

void
wpd_srand();

int
wpd_rand();

#endif

