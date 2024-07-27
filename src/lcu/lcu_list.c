#include <string.h>
#include <stdlib.h>
#include "lcu.h"
#include "lcu_list.h"

#define LIST(x) ((list_t *)x)

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
} list_t;

static list_item_t *lcu_helper_new_list_item(size_t size, void *value);
static list_item_t *lcu_helper_get_list_item(list_t *handle, int i);

lcu_list_t lcu_list_create()
{
    list_t *handle = (list_t *) malloc(sizeof(list_t));
    if (handle == NULL)
        return NULL;
    handle->size = 0;
    handle->head = NULL;
    handle->tail = NULL;

    return ((lcu_list_t)handle);
}

size_t lcu_list_get_size(lcu_list_t handle)
{
    return LIST(handle)->size;
}

int lcu_list_insert_front(lcu_list_t handle, size_t size, void *value)
{
    list_item_t *temp = lcu_helper_new_list_item(size, value);
    if (temp == NULL)
        return -1;

    if (LIST(handle)->size == 0)
    {
        temp->next = NULL;
        temp->prev = NULL;
        LIST(handle)->tail = temp;
    }
    else
    {
        temp->next = LIST(handle)->head;
        temp->prev = NULL;
        LIST(handle)->head->prev = temp;
    }
    LIST(handle)->head = temp;
    LIST(handle)->size++;

    return 0;
}

int lcu_list_insert_back(lcu_list_t handle, size_t size, void *value)
{
    list_item_t *temp = lcu_helper_new_list_item(size, value);
    if (temp == NULL)
        return -1;

    if (LIST(handle)->size == 0)
    {
        temp->next = NULL;
        temp->prev = NULL;
        LIST(handle)->head = temp;
    }
    else
    {
        temp->next = NULL;
        temp->prev = LIST(handle)->tail;
        LIST(handle)->tail->next = temp;
    }
    LIST(handle)->tail = temp;
    LIST(handle)->size++;

    return 0;
}

int lcu_list_insert_at_i(lcu_list_t handle, int i, size_t size, void *value)
{
    if (i == 0)
        return lcu_list_insert_front(handle, size, value);
    if (i < 0 || i >= (int)(LIST(handle)->size - 1))
        return lcu_list_insert_back(handle, size, value);

    list_item_t *temp = lcu_helper_get_list_item(LIST(handle), i);
    if (temp == NULL)
        return -1;

    list_item_t *i_next = temp;
    list_item_t *i_prev = temp->prev;

    temp = NULL;
    temp = lcu_helper_new_list_item(size, value);
    if (temp == NULL)
        return -1;

    i_next->prev = temp;
    i_prev->next = temp;
    temp->next = i_next;
    temp->prev = i_prev;

    LIST(handle)->size++;

    return 0;
}

void *lcu_list_peek_front(lcu_list_t handle)
{
    return (LIST(handle)->head->value);
}

void *lcu_list_peek_back(lcu_list_t handle)
{
    return (LIST(handle)->tail->value);
}

void *lcu_list_peek_at_i(lcu_list_t handle, int i)
{
    if (i >= (int)LIST(handle)->size)
        return NULL;

    if (i == 0)
        return lcu_list_peek_front(handle);
    if (i < 0 || i == (int)(LIST(handle)->size - 1))
        return lcu_list_peek_back(handle);

    list_item_t *temp = lcu_helper_get_list_item(LIST(handle), i);
    if (temp == NULL)
        return NULL;

    return temp->value;
}

int lcu_list_remove_front(lcu_list_t handle)
{
    list_item_t *temp = LIST(handle)->head;

    temp->size = 0;
    free(temp->value);
    temp->value = NULL;

    if (LIST(handle)->size == 1)
    {
        free(temp);
        LIST(handle)->head = NULL;
        LIST(handle)->tail = NULL;
    }
    else
    {
        temp = temp->next;
        temp->prev->next = NULL;
        free(temp->prev);
        temp->prev = NULL;

        LIST(handle)->head = temp;
    }
    LIST(handle)->size--;

    return 0;
}

int lcu_list_remove_back(lcu_list_t handle)
{    
    list_item_t *temp = LIST(handle)->tail;

    temp->size = 0;
    free(temp->value);
    temp->value = NULL;

    if (LIST(handle)->size == 1)
    {
        free(temp);
        LIST(handle)->head = NULL;
        LIST(handle)->tail = NULL;
    }
    else
    {
        temp = temp->prev;
        temp->next->prev = NULL;
        free(temp->next);
        temp->next = NULL;

        LIST(handle)->tail = temp;
    }
    LIST(handle)->size--;

    return 0;
}

int lcu_list_remove_at_i(lcu_list_t handle, int i)
{
    if (i >= (int)LIST(handle)->size)
        return -2;

    if (i == 0)
        return lcu_list_remove_front(handle);
    if (i < 0 || i == (int)(LIST(handle)->size - 1))
        return lcu_list_remove_back(handle);

    list_item_t *temp = lcu_helper_get_list_item(LIST(handle), i);
    if (temp == NULL)
        return -1;

    temp->size = 0;
    free(temp->value);
    temp->value = NULL;

    list_item_t *i_next = temp->next;
    list_item_t *i_prev = temp->prev;
    i_next->prev = i_prev;
    i_prev->next = i_next;

    free(temp);
    temp = NULL;

    LIST(handle)->size--;

    return 0;
}

void lcu_list_destroy(lcu_list_t *handle)
{    
    for (size_t i = 0; i < LIST(*handle)->size; i++)
        IGNORE_RET(lcu_list_remove_back(*handle));

    free(*handle);
    *handle = NULL;
}

list_item_t *lcu_helper_new_list_item(size_t size, void *value)
{
    list_item_t *temp = NULL;
    temp = (list_item_t *) malloc(sizeof(list_item_t));
    if (temp == NULL)
        return NULL;

    temp->value = malloc(size);
    if (temp->value == NULL)
    {
        free(temp);
        temp = NULL;

        return NULL;
    }
    temp->size = size;
    IGNORE_RET(memcpy(temp->value, value, size));

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
