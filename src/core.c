#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#include "core.h"

#define PATH_SEPARATOR "/"

const char* error_strings[] = {
    "WPD_ERROR_SUCCESS",
    "WPD_ERROR_FAILURE",
    "WPD_ERROR_UNKNOWN_FILENAME",
    "WPD_ERROR_UNKNOWN_EXTENSION",
    "WPD_ERROR_NULL_PARAM",
    "WPD_ERROR_TODO"
};

void wpd_exit(wpd_error_t exit_code)
{
    exit(exit_code);
}

const char*
wpd_error_str(wpd_error_t error)
{
    return error_strings[error];
}

char*
wpd_get_extension(const char *path)
{
    char *temp = strrchr(path, '.');
    return (temp) ? temp + 1 : NULL;
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

uint32_t
wpd_min(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

char*
wpd_path_join(const char *a, const char *b)
{
    size_t len = strlen(a) + strlen(PATH_SEPARATOR) + strlen(b) + 1;
    char *result = malloc(len);
    snprintf(result, len, "%s%s%s", a, PATH_SEPARATOR, b);

    return result;
}


char*
wpd_strdup_lower(const char *data)
{
    char *lower = strdup(data);
    for (int i = 0; lower[i]; i++)
        lower[i] = tolower(lower[i]);
    return lower;
}

