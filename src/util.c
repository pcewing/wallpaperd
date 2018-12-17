#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "util.h"

void wpd_exit(int exit_code)
{
    exit(exit_code);
}

void die(const char * message)
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
getfilename(const char *pathname)
{
    return getafter(pathname, '/');
}

char*
getextension(const char *filename)
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

