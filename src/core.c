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
wpd_error_str(wpd_error_t error)
{
    return error_strings[error];
}

char*
wpd_get_extension(const char *filename)
{
    char *temp, *result = NULL;

    temp = strrchr(filename, '.');
    if (temp) {
        result = temp + 1;
    }

    return result;
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

