#ifndef CONF_DEF_H
#define CONF_DEF_H

#include <stddef.h>

typedef struct conf {
    char *log_dir;
    char *ap_name;
    size_t tpool_threads;
} conf_t;

conf_t *get_conf();

#endif // CONF_DEF_H