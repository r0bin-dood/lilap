#ifndef LCU_LOG_H
#define LCU_LOG_H

#define LCU_STDOUT "stdout"
#define LCU_STDERR "stderr"

void lcu_logger_create(const char *out);
void lcu_logger_print(const char *fmt, ...);
void lcu_logger_destroy();

#endif // LCU_LOG_H