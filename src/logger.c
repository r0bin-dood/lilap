#include <stdio.h>
#include <stdarg.h>

#include "logger.h"

enum {
    INFO,
    WARN,
    ERROR
};

static void _log(int level, char *format, va_list args);

void log_info(char *format, ...)
{
    va_list args;
    va_start(args, format);
    _log(INFO, format, args);
    va_end(args);
}

void log_warn(char *format, ...)
{
    va_list args;
    va_start(args, format);
    _log(WARN, format, args);
    va_end(args);
}

void log_error(char *format, ...)
{
    va_list args;
    va_start(args, format);
    _log(ERROR, format, args);
    va_end(args);
}

void _log(int level, char *format, va_list args)
{
    switch (level)
    {
        default:
        case INFO:
        {
            printf("[INFO] ");
            break;
        }
        case WARN:
        {
            printf("[WARN] ");
            break;
        }
        case ERROR:
        {
            printf("[ERROR] ");
            break;
        }
    }
    vprintf(format, args);
    printf("\n");
}