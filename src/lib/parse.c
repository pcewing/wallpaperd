#include "parse.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

wpd_error_t parse_bool(const char *data, bool *result) {
    static const char *t[] = {"true", "on", "yes", "y", NULL};
    static const char *f[] = {"false", "off", "no", "n", NULL};

    char *lower = wpd_strdup_lower(data);

    wpd_error_t error = WPD_ERROR_GLOBAL_SUCCESS;

    if (strmatch(t, lower))
        *result = true;
    else if (strmatch(f, lower))
        *result = false;
    else
        error = WPD_ERROR_PARSE_BOOL_INVALID;

    free(lower);
    return error;
}

wpd_error_t parse_uint32(const char *data, uint32_t *result) {
    char *end;
    errno = 0;
    const uint64_t ret = strtoull(data, &end, 10);

    if (end == data) {
        return WPD_ERROR_PARSE_U32_NOT_A_DECIMAL;
    }

    if (*end != '\0') {
        return WPD_ERROR_PARSE_U32_TRAILING_CHARS;
    }

    if (ret == ULLONG_MAX && ERANGE == errno) {
        return WPD_ERROR_PARSE_U32_STRTOULL_OUT_OF_RANGE;
    }

    if (ret > ULONG_MAX) {
        return WPD_ERROR_PARSE_U32_OVERFLOW;
    }

    (*result) = (uint32_t)ret;
    return WPD_ERROR_GLOBAL_SUCCESS;
}
