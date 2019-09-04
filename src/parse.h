#ifndef PARSE_H
#define PARSE_H

#include "core.h"

/**
 * Attempts to parse a boolean value from an input string.
 **/
wpd_error_t parse_bool(const char *data, bool *result);

/**
 * Attempts to parse an unsigned 32 bit integer value from an input string.
 **/
wpd_error_t parse_uint32(const char *data, uint32_t *result);

#endif // PARSE_H
