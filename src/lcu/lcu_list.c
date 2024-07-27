#include <string.h>
#include <stdlib.h>
#include "lcu_list.h"

typedef struct list_item {
    size_t size;
    void *value;
    struct list_item *prev;
    struct list_item *next;
} list_item_t;

typedef struct list {
    size_t size;
    list_item_t *head;
    list_item_t *tail;
    lcu_cleanup_callback cleanup_func;
} list_t;

static list_item_t *lcu_helper_new_list_item(void *value);
static list_item_t *lcu_helper_get_list_item(list_t *handle, int i);

lcu_list_t lcu_list_create(lcu_cleanup_callback cleanup_func)
{
    list_t *handle = (list_t *) malloc(sizeof(list_t));
    if (handle == NULL)
        return NULL;
    handle->size = 0;
    handle->head = NULL;
    handle->tail = NULL;
    handle->cleanup_func = cleanup_func;

    return ((lcu_list_t)handle);
}

size_t lcu_list_get_size(lcu_list_t handle)
{
    list_t *list = (list_t *)handle;
    return list->size;
}

int lcu_list_insert_front(lcu_list_t handle, void *value)
{
    list_t *list = (list_t *)handle;

    list_item_t *temp = lcu_helper_new_list_item(value);
    if (temp == NULL)
        return -1;

    if (list->size == 0)
    {
        temp->next = NULL;
        temp->prev = NULL;
        list->tail = temp;
    }
    else
    {
        temp->next = list->head;
        temp->prev = NULL;
        list->head->prev = temp;
    }
    list->head = temp;
    list->size++;

    return 0;
}

int lcu_list_insert_back(lcu_list_t handle, void *value)
{
    list_t *list = (list_t *)handle;

    list_item_t *temp = lcu_helper_new_list_item(value);
    if (temp == NULL)
        return -1;

    if (list->size == 0)
    {
        temp->next = NULL;
        temp->prev = NULL;
        list->head = temp;
    }
    else
    {
        temp->next = NULL;
        temp->prev = list->tail;
        list->tail->next = temp;
    }
    list->tail = temp;
    list->size++;

    return 0;
}

int lcu_list_insert_at_i(lcu_list_t handle, int i, void *value)
{
    list_t *list = (list_t *)handle;

    if (i == 0)
        return lcu_list_insert_front(handle, value);
    if (i < 0 || i >= (int)(list->size - 1))
        return lcu_list_insert_back(handle, value);

    list_item_t *temp = lcu_helper_get_list_item(list, i);
    if (temp == NULL)
        return -1;

    list_item_t *i_next = temp;
    list_item_t *i_prev = temp->prev;

    temp = NULL;
    temp = lcu_helper_new_list_item(value);
    if (temp == NULL)
        return -1;

    i_next->prev = temp;
    i_prev->next = temp;
    temp->next = i_next;
    temp->prev = i_prev;

    list->size++;

    return 0;
}

void *lcu_list_peek_front(lcu_list_t handle)
{
    list_t *list = (list_t *)handle;
    return (list->head->value);
}

void *lcu_list_peek_back(lcu_list_t handle)
{
    list_t *list = (list_t *)handle;
    return (list->tail->value);
}

void *lcu_list_peek_at_i(lcu_list_t handle, int i)
{
    list_t *list = (list_t *)handle;

    if (i >= (int)list->size)
        return NULL;

    if (i == 0)
        return lcu_list_peek_front(handle);
    if (i < 0 || i == (int)(list->size - 1))
        return lcu_list_peek_back(handle);

    list_item_t *temp = lcu_helper_get_list_item(list, i);
    if (temp == NULL)
        return NULL;

    return temp->value;
}

int lcu_list_remove_front(lcu_list_t handle)
{
    list_t *list = (list_t *)handle;

    list_item_t *temp = list->head;

    temp->size = 0;
    if (list->cleanup_func != NULL)
        (*list->cleanup_func)(temp->value);
    temp->value = NULL;

    if (list->size == 1)
    {
        free(temp);
        list->head = NULL;
        list->tail = NULL;
    }
    else
    {
        temp = temp->next;
        temp->prev->next = NULL;
        free(temp->prev);
        temp->prev = NULL;

        list->head = temp;
    }
    list->size--;

    return 0;
}

int lcu_list_remove_back(lcu_list_t handle)
{    
    list_t *list = (list_t *)handle;

    list_item_t *temp = list->tail;

    temp->size = 0;
    if (list->cleanup_func != NULL)
        (*list->cleanup_func)(temp->value);
    temp->value = NULL;

    if (list->size == 1)
    {
        free(temp);
        list->head = NULL;
        list->tail = NULL;
    }
    else
    {
        temp = temp->prev;
        temp->next->prev = NULL;
        free(temp->next);
        temp->next = NULL;

        list->tail = temp;
    }
    list->size--;

    return 0;
}

int lcu_list_remove_at_i(lcu_list_t handle, int i)
{
    list_t *list = (list_t *)handle;

    if (i >= (int)list->size)
        return -2;

    if (i == 0)
        return lcu_list_remove_front(handle);
    if (i < 0 || i == (int)(list->size - 1))
        return lcu_list_remove_back(handle);

    list_item_t *temp = lcu_helper_get_list_item(list, i);
    if (temp == NULL)
        return -1;

    temp->size = 0;
    if (list->cleanup_func != NULL)
        (*list->cleanup_func)(temp->value);
    temp->value = NULL;

    list_item_t *i_next = temp->next;
    list_item_t *i_prev = temp->prev;
    i_next->prev = i_prev;
    i_prev->next = i_next;

    free(temp);
    temp = NULL;

    list->size--;

    return 0;
}

void lcu_list_destroy(lcu_list_t *handle)
{
    list_t **list = (list_t **)handle;

    for (size_t i = 0; i < (*list)->size; i++)
        IGNORE_RET(lcu_list_remove_back(*handle));

    free(*handle);
    *handle = NULL;
}

list_item_t *lcu_helper_new_list_item(void *value)
{
    list_item_t *temp = (list_item_t *) malloc(sizeof(list_item_t));
    if (temp == NULL)
        return NULL;

    temp->value = value;

    return temp;
}

list_item_t *lcu_helper_get_list_item(list_t *handle, int i)
{
    list_item_t *temp = handle->head;

    for (int j = 0; j < (int) handle->size; j++)
    {
        if (j == i)
            return temp;
        temp = temp->next;
    }

    return NULL;
}
