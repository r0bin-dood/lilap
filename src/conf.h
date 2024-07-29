#ifndef CONF_H
#define CONF_G

#include <stddef.h>

typedef struct conf {
    char log_dir[256]; // "/var/run"
    char ap_name[32]; // "lilap AP"
    size_t tpool_threads; // 8
} conf_t;

void conf_parse(conf_t *conf, const char *file);

#endif // CONF_H