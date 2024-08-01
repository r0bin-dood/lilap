#include "conf_def.h"

static conf_t conf = {
    .log_dir = "stdout",
    .ap_name = "lilap AP",
    .tpool_threads = 8
};

conf_t *get_conf()
{
    return &conf;
}