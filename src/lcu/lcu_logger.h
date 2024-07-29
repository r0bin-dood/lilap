#ifndef LCU_LOG_H
#define LCU_LOG_H

#include <stddef.h>

#define LCU_STDOUT "stdout"
#define LCU_STDERR "stderr"

typedef void * lcu_logger_t;

void lcu_logger_create(const char *dest);
void lcu_logger_print(const char *fmt, ...);
void lcu_logger_destroy();

#endif // LCU_LOG_H