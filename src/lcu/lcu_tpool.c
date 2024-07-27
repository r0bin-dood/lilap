#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "lcu.h"
#include "lcu_fifo.h"
#include "lcu_tpool.h"

typedef struct tpool {
    size_t num_threads;
    pthread_t *worker_id_arr;
    lcu_fifo_t worker_fifo;
    pthread_mutex_t worker_fifo_mutex;
} tpool_t;

typedef struct worker {
    pthread_t worker_id;
    pthread_cond_t work_cond;
    pthread_mutex_t work_cond_mutex;
    lcu_tpool_worker_func_t work_func;
    void *work_func_args;
} worker_t;

typedef struct worker_data {
    tpool_t *tpool;
    worker_t *worker;
} worker_data_t;

static void *lcu_helper_worker_func(void *arg);
static void lcu_helper_worker_cleanup(void *arg);

lcu_tpool_t lcu_tpool_create(size_t num_threads)
{
    tpool_t *tpool = (tpool_t *) malloc(sizeof(tpool_t));
    if (tpool == NULL)
        return NULL;
    
    pthread_mutex_init(&tpool->worker_fifo_mutex, NULL);
    
    tpool->worker_fifo = lcu_fifo_create();
    if (tpool->worker_fifo == NULL)
    {
        free(tpool);
        return NULL;
    }

    tpool->num_threads = num_threads;
    tpool->worker_id_arr = (pthread_t *) malloc(sizeof(pthread_t) * num_threads);
    if (tpool->worker_id_arr == NULL)
        goto error;

    for (size_t i = 0; i < num_threads; i++)
    {
        worker_t *worker = (worker_t *) malloc(sizeof(worker_t));
        if (worker == NULL)
            goto error;
        worker->work_func = NULL;
        worker->work_func_args = NULL;

        pthread_cond_init(&worker->work_cond, NULL);
        pthread_mutex_init(&worker->work_cond_mutex, NULL);
        
        worker_data_t *data = (worker_data_t *) malloc(sizeof(worker_data_t));
        if (data == NULL)
            goto error;
        data->tpool = tpool;
        data->worker = worker;

        int ret = pthread_create(&worker->worker_id, NULL, &lcu_helper_worker_func, (void *)data);
        if (ret != 0)
            goto error;
        tpool->worker_id_arr[i] = worker->worker_id;

        ret = lcu_fifo_push(tpool->worker_fifo, sizeof(worker_t), (void *)&worker);
        if (ret != 0)
            goto error;
    }

    return tpool;

    error:
    lcu_fifo_destroy(&tpool->worker_fifo);
    free(tpool);
    return NULL;
}

size_t lcu_tpool_get_total_size(lcu_tpool_t handle)
{
    tpool_t *tpool = (tpool_t *)handle;
    return tpool->num_threads;
}

size_t lcu_tpool_get_available_size(lcu_tpool_t handle)
{
    tpool_t *tpool = (tpool_t *)handle;
    return lcu_fifo_get_size(tpool->worker_fifo);
}

void lcu_tpool_grow(lcu_tpool_t handle, size_t num_threads)
{
    UNUSED(handle);
    UNUSED(num_threads);
}

int lcu_tpool_add_work(lcu_tpool_t handle, lcu_tpool_worker_func_t func, void *args)
{
    tpool_t *tpool = (tpool_t *)handle;

    if (lcu_tpool_get_available_size(tpool) == 0)
        return -1;

    IGNORE_RET(pthread_mutex_lock(&tpool->worker_fifo_mutex));
    worker_t *worker = (worker_t *) lcu_fifo_peek(tpool->worker_fifo);
    worker->work_func = func;
    worker->work_func_args = args;
    lcu_fifo_pop(tpool->worker_fifo);
    IGNORE_RET(pthread_mutex_unlock(&tpool->worker_fifo_mutex));
    pthread_cond_signal(&worker->work_cond);

    return 0;
}

void lcu_tpool_destroy(lcu_tpool_t *handle)
{
    tpool_t **tpool = (tpool_t **)handle;

    for (size_t i = 0; i < (*tpool)->num_threads; i++)
    {
        pthread_cancel((*tpool)->worker_id_arr[i]);
        pthread_join((*tpool)->worker_id_arr[i], NULL);
    }
    IGNORE_RET(pthread_mutex_lock(&(*tpool)->worker_fifo_mutex));
    lcu_fifo_destroy(&(*tpool)->worker_fifo);
    IGNORE_RET(pthread_mutex_unlock(&(*tpool)->worker_fifo_mutex));
    pthread_mutex_destroy(&(*tpool)->worker_fifo_mutex);
    free((*tpool)->worker_id_arr);
    free((*tpool));
    *tpool = NULL;
}

void lcu_helper_worker_cleanup(void *arg)
{
    worker_data_t *data = (worker_data_t *)arg;
    worker_t *worker = data->worker;

    pthread_cond_destroy(&worker->work_cond);
    pthread_mutex_destroy(&worker->work_cond_mutex);

    free(worker);
    free(data);
}

void *lcu_helper_worker_func(void *arg)
{
    worker_data_t *data = (worker_data_t *)arg;
    worker_t *worker = data->worker;
    tpool_t *tpool = data->tpool;

    pthread_cleanup_push(lcu_helper_worker_cleanup, arg);

    while (true)
    {
        IGNORE_RET(pthread_mutex_lock(&worker->work_cond_mutex));
        IGNORE_RET(pthread_cond_wait(&worker->work_cond, &worker->work_cond_mutex));
        IGNORE_RET(pthread_mutex_unlock(&worker->work_cond_mutex));

        (*worker->work_func)(worker->work_func_args);
        worker->work_func = NULL;
        worker->work_func_args = NULL;

        IGNORE_RET(pthread_mutex_lock(&tpool->worker_fifo_mutex));
        lcu_fifo_push(tpool->worker_fifo, sizeof(*worker), (void *)worker);
        IGNORE_RET(pthread_mutex_unlock(&tpool->worker_fifo_mutex));
    }

    pthread_cleanup_pop(true);

    return NULL;
}
