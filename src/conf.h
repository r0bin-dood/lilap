#ifndef CONF_H
#define CONF_G

#include <stddef.h>

typedef struct conf {
    char *log_dir;
    char *ap_name;
    size_t tpool_threads;
} conf_t;

void conf_parse(conf_t *conf, const char *file);

#endif // CONF_H