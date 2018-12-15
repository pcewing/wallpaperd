#ifndef UTIL_H
#define UTIL_H

#include <string.h>

#define UNUSED(x) (void)(x)

#define WPD_EXIT_SUCCESS 0
#define WPD_EXIT_FAILURE 1

void wpd_exit(int exit_code);
void die(const char * message);

char*
getfilename(const char *pathname);

char*
getextension(const char *filename);

#endif

