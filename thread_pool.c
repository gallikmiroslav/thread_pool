/*
 * MIT License
 *
 * Copyright (c) 2025 Miroslav Gallik
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include "thread_pool.h"

typedef struct thread_pool_s thread_pool_t;

typedef struct
{
    tp_func_t func;
    void *arg;
} tp_task_t;

typedef struct
{
    thread_pool_t *thread_pool;
    pthread_t thr;
    int tid;
    void *user_data;
} tp_thread_t;

struct thread_pool_s
{
    // Constants
    size_t pool_size;
    size_t queue_size;

    tp_thread_t *thr_pool;

    // Protected by queue_rd_mtx
    size_t queue_rd_pos;
    pthread_mutex_t queue_rd_mtx;

    // Protected by queue_wr_mtx
    size_t queue_wr_pos;
    pthread_mutex_t queue_wr_mtx;

    // Provides synchronization between producer and consumer
    sem_t sem_queue;
    sem_t sem_queue_free;

    // Access synchronized through sem_queue and sem_queue_free
    tp_task_t *queue;
};

void *worker_thread(void *arg)
{
    tp_thread_t *thr = (tp_thread_t *) arg;
    thread_pool_t *tp = thr->thread_pool;
    tp_task_t *task;
    tp_func_t t_func;
    void *t_arg;

    for (;;)
    {
        // wait for a new task
        sem_wait(&tp->sem_queue);

        // get task
        pthread_mutex_lock(&tp->queue_rd_mtx);
        task = tp->queue + tp->queue_rd_pos;
        tp->queue_rd_pos++;
        if (tp->queue_rd_pos == tp->queue_size)
            tp->queue_rd_pos = 0;
        pthread_mutex_unlock(&tp->queue_rd_mtx);

        // free queue space
        t_func = task->func;
        t_arg = task->arg;
        sem_post(&tp->sem_queue_free);

        // end worker if NULL
        if (!t_func)
            break;

        // execute task
        t_func(t_arg, thr->tid, thr->user_data);
    }
    return NULL;
}


// ---------------------------------------------------------------------------------------------------------------------

void *
tp_init(size_t pool_size, size_t queue_size)
{
    assert(pool_size > 0);
    assert(queue_size > 0);

    thread_pool_t *tp = (thread_pool_t *) malloc(sizeof(thread_pool_t)
                                                 + pool_size * sizeof(tp_thread_t)
                                                 + queue_size * sizeof(tp_task_t));

    if (tp)
    {
        tp->pool_size = pool_size;
        tp->thr_pool = (tp_thread_t *) (tp + 1);
        tp->queue_size = queue_size;
        tp->queue = (tp_task_t *) (tp->thr_pool + pool_size);

        tp->queue_rd_pos = 0;
        pthread_mutex_init(&tp->queue_rd_mtx, NULL);
        tp->queue_wr_pos = 0;
        pthread_mutex_init(&tp->queue_wr_mtx, NULL);

        sem_init(&tp->sem_queue, 0, 0);
        sem_init(&tp->sem_queue_free, 0, queue_size);

        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        for (int i = 0; i < pool_size; i++)
        {
            tp_thread_t *t = tp->thr_pool + i;
            t->thread_pool = tp;
            t->tid = i;
            t->user_data = NULL;
            pthread_create(&t->thr, &attr, worker_thread, t);
        }

        pthread_attr_destroy(&attr);
    }

    return tp;
}

int
tp_add_task(void *thread_pool, tp_func_t func, void *arg, int no_block)
{
    assert(thread_pool);
    thread_pool_t *tp = (thread_pool_t *) thread_pool;
    int ret;

    // check for space in the queue
    if (no_block)
        ret = sem_trywait(&tp->sem_queue_free);
    else
        ret = sem_wait(&tp->sem_queue_free);

    if (!ret)
    {
        pthread_mutex_lock(&tp->queue_wr_mtx);
        tp->queue[tp->queue_wr_pos].func = func;
        tp->queue[tp->queue_wr_pos].arg = arg;
        tp->queue_wr_pos++;
        if (tp->queue_wr_pos == tp->queue_size)
            tp->queue_wr_pos = 0;
        pthread_mutex_unlock(&tp->queue_wr_mtx);

        // submit a task to the pool
        sem_post(&tp->sem_queue);
    }
    return ret;
}

void
tp_done(void *thread_pool)
{
    assert(thread_pool);
    thread_pool_t *tp = (thread_pool_t *) thread_pool;
    void *ret;

    for (int i = 0; i < tp->pool_size; i++)
        tp_add_task(tp, NULL, NULL, 0);
    for (int i = 0; i < tp->pool_size; i++)
        pthread_join((tp->thr_pool + i)->thr, &ret);

    pthread_mutex_destroy(&tp->queue_rd_mtx);
    pthread_mutex_destroy(&tp->queue_wr_mtx);

    sem_destroy(&tp->sem_queue);
    sem_destroy(&tp->sem_queue_free);

    free(thread_pool);
}