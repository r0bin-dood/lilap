#ifndef LCU_LIST_H
#define LCU_LIST_H

#include <stddef.h>
#include "lcu.h"

typedef void * lcu_list_t;

lcu_list_t lcu_list_create(lcu_cleanup_callback cleanup_func);
size_t lcu_list_get_size(lcu_list_t handle);
int lcu_list_insert_front(lcu_list_t handle, void *value);
int lcu_list_insert_back(lcu_list_t handle, void *value);
int lcu_list_insert_at_i(lcu_list_t handle, int i, void *value);
void *lcu_list_peek_front(lcu_list_t handle);
void *lcu_list_peek_back(lcu_list_t handle);
void *lcu_list_peek_at_i(lcu_list_t handle, int i);
int lcu_list_remove_front(lcu_list_t handle);
int lcu_list_remove_back(lcu_list_t handle);
int lcu_list_remove_at_i(lcu_list_t handle, int i);
void lcu_list_destroy(lcu_list_t *handle);

#endif // LCU_LIST_H