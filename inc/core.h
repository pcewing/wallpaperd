#ifndef CORE_H
#define CORE_H

#include <string.h>
#include <stdint.h>

#define UNUSED(x) (void)(x)

typedef int wpd_error_t;

#define WPD_ERROR_SUCCESS 0x0000
#define WPD_ERROR_FAILURE 0x0001

#define WPD_ERROR_UNKNOWN_FILENAME 0x0002
#define WPD_ERROR_UNKNOWN_EXTENSION 0x0003
#define WPD_ERROR_NULL_PARAM 0x0004
#define WPD_ERROR_TODO 0x0005
#define WPD_ERROR_NO_COMPATIBLE_IMAGE 0x0006

#define WPD_ERROR_DATABASE_TABLE_CREATION_FAILURE 0x0007

void
wpd_exit(int exit_code);

const char*
wpd_error_str(wpd_error_t error);

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

#endif // CORE_H

