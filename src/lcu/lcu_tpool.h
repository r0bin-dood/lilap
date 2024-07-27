#ifndef LCU_TPOOL_H
#define LCU_TPOOL_H

#include <stddef.h>

typedef void * lcu_tpool_t;
typedef void (*lcu_tpool_worker_func_t)(void *);

lcu_tpool_t lcu_tpool_create(size_t num_threads);
size_t lcu_tpool_get_total_size(lcu_tpool_t handle);
size_t lcu_tpool_get_available_size(lcu_tpool_t handle);
void lcu_tpool_grow(lcu_tpool_t handle, size_t num_threads);
int lcu_tpool_add_work(lcu_tpool_t handle, lcu_tpool_worker_func_t func, void *args);
void lcu_tpool_destroy(lcu_tpool_t *handle);

#endif // LCU_TPOOL_H