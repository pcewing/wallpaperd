#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "core.h"

#define PATH_SEPARATOR "/"

void wpd_exit(wpd_error_t exit_code) { exit(exit_code); }

char *wpd_get_extension(const char *path) {
    char *temp = strrchr(path, '.');
    return (temp) ? temp + 1 : NULL;
}

unsigned int wpd_sleep(unsigned int seconds) { return sleep(seconds); }
unsigned int wpd_usleep(unsigned int seconds) { return usleep(seconds); }

uint64_t get_timestamp_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (uint64_t)1000000 * (uint64_t)tv.tv_sec + (uint64_t)tv.tv_usec;
}

void wpd_srand() {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);

    srand(spec.tv_nsec);
}

int wpd_rand() { return rand(); }

uint32_t wpd_min(uint32_t a, uint32_t b) { return (a < b) ? a : b; }

char *wpd_path_join(const char *a, const char *b) {
    size_t len = strlen(a) + strlen(PATH_SEPARATOR) + strlen(b) + 1;
    char *result = malloc(len);
    snprintf(result, len, "%s%s%s", a, PATH_SEPARATOR, b);

    return result;
}

char *wpd_strdup_lower(const char *data) {
    char *lower = strdup(data);
    for (int i = 0; lower[i]; i++)
        lower[i] = tolower(lower[i]);
    return lower;
}

bool strmatch(const char **haystack, const char *needle) {
    for (int i = 0; haystack[i]; i++)
        if (strcmp(needle, haystack[i]) == 0)
            return true;
    return false;
}

char *wpd_strerror(int errnum) { return strerror(errnum); }
