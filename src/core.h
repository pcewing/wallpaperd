#ifndef CORE_H
#define CORE_H

#include <string.h>
#include <stdint.h>

#include "error.g.h"

#define UNUSED(x) (void)(x)

void
wpd_exit(int exit_code);

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

uint32_t
wpd_min(uint32_t a, uint32_t b);

// TODO: document this
// The string returned must be manually freed by the caller
char*
wpd_path_join(const char *a, const char *b);

// TODO: document this
// The string returned must be manually freed by the caller
char*
wpd_strdup_lower(const char *data);

#endif // CORE_H

