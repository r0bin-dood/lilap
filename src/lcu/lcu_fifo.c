#include "lcu_fifo.h"
#include "lcu_list.h"

lcu_fifo_t lcu_fifo_create(lcu_cleanup_callback cleanup_func)
{
    return lcu_list_create(cleanup_func);
}

size_t lcu_fifo_get_size(lcu_fifo_t handle)
{
    return lcu_list_get_size(handle);
}

int lcu_fifo_push(lcu_fifo_t handle, void *value)
{
    return lcu_list_insert_back(handle, value);
}

void *lcu_fifo_peek(lcu_fifo_t handle)
{
    return lcu_list_peek_front(handle);
}

int lcu_fifo_pop(lcu_fifo_t handle)
{
    return lcu_list_remove_front(handle);
}

void lcu_fifo_destroy(lcu_fifo_t *handle)
{
    lcu_list_destroy(handle);
}
