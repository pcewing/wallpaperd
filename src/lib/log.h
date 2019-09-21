#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdint.h>

typedef uint8_t log_level_t;

#define LOGLEVEL_TRACE 0
#define LOGLEVEL_DEBUG 1
#define LOGLEVEL_INFO 2
#define LOGLEVEL_WARN 3
#define LOGLEVEL_ERROR 4

void LOG(const log_level_t level, const char *message, ...);

#define LOGTRACE(message, args...) LOG(LOGLEVEL_TRACE, message, ##args)
#define LOGDEBUG(message, args...) LOG(LOGLEVEL_DEBUG, message, ##args)
#define LOGINFO(message, args...) LOG(LOGLEVEL_INFO, message, ##args)
#define LOGWARN(message, args...) LOG(LOGLEVEL_WARN, message, ##args)
#define LOGERROR(message, args...) LOG(LOGLEVEL_ERROR, message, ##args)

#endif // LOG_H
