#ifndef LCU_FIFO_H
#define LCU_FIFO_H

#include <stddef.h>
#include "lcu.h"

typedef void * lcu_fifo_t;

lcu_fifo_t lcu_fifo_create(lcu_cleanup_callback cleanup_func);
size_t lcu_fifo_get_size(lcu_fifo_t handle);
int lcu_fifo_push(lcu_fifo_t handle, void *value);
void *lcu_fifo_peek(lcu_fifo_t handle);
int lcu_fifo_pop(lcu_fifo_t handle);
void lcu_fifo_destroy(lcu_fifo_t *handle);

#endif // LCU_FIFO_H