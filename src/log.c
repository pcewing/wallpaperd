#include "log.h"

#include <stdio.h>

const char*
log_level_str(const log_level_t level)
{
    const char * result = NULL;
    switch(level)
    {
        case LOGLEVEL_TRACE: result = "TRACE"; break;
        case LOGLEVEL_DEBUG: result = "DEBUG"; break;
        case LOGLEVEL_INFO:  result = "INFO"; break;
        case LOGLEVEL_WARN:  result = "WARN"; break;
        case LOGLEVEL_ERROR: result = "ERROR"; break;
        default:             result = "UNKNOWN"; break;
    };
    return result;
}

void LOG(const log_level_t level, const char* format, ...)
{
    printf("[%s] ", log_level_str(level));

    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);

    printf("\n");
}
