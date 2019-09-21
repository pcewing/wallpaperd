#include "log.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#define LOGLEVEL LOGLEVEL_DEBUG

static bool logs_suppressed = false;

static const char *log_level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR",
};

const char *log_level_str(const log_level_t level) {
    assert(level <= LOGLEVEL_ERROR);
    return log_level_strings[level];
}

void suppress_logs() {
    logs_suppressed = true;
}

void LOG(const log_level_t level, const char *format, ...) {
    if (logs_suppressed || level < LOGLEVEL) {
        return;
    }

    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);

    printf("[%s] ", log_level_str(level));

    va_list args;
    va_start(args, format);

    vprintf(format, args);

    va_end(args);

    printf("\n");
}
