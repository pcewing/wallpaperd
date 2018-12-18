#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "core.h"

const char* error_strings[] = {
    "WPD_ERROR_SUCCESS",
    "WPD_ERROR_FAILURE",
    "WPD_ERROR_UNKNOWN_FILENAME",
    "WPD_ERROR_UNKNOWN_EXTENSION",
    "WPD_ERROR_NULL_PARAM",
    "WPD_ERROR_TODO"
};

void wpd_exit(int exit_code)
{
    exit(exit_code);
}

const char*
wpd_error_str(wpd_error error)
{
    return error_strings[error];
}

void wpd_die(const char * message)
{
    printf("%s\n", message);
    exit(1);
}

char*
getafter(const char *str, const char c)
{
    char *temp, *result = NULL;

    temp = strrchr(str, c);
    if (temp) {
        result = temp + 1;
    }

    return result;
}

char*
wpd_get_filename(const char *pathname)
{
    return getafter(pathname, '/');
}

char*
wpd_get_extension(const char *filename)
{
    return getafter(filename, '.');
}

unsigned int
wpd_sleep(unsigned int seconds)
{
    return sleep(seconds);
}

void
wpd_srand()
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    srand(spec.tv_nsec);
}

int
wpd_rand()
{
    return rand();
}

