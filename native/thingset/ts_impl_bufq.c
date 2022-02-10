/*
 * Copyright (c) 2019 chrismerck@gmail.com
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Modified from https://github.com/chrismerck/rpa_queue
 */

/**
 * @file
 * @brief Native implementation of ThingSet communication buffer queue.
 */

#include "../../src/thingset_env.h"
#include "../../src/thingset_time.h"

#include "../../src/ts_macro.h"
#include "../../src/ts_log.h"

#include "ts_impl_bufq.h"

#define TS_IMPL_BUFQ_LOGD(msg, bufq)   \
    TS_LOGD("BUFQ: %s on queue 0x%" PRIXPTR " %s", __func__, (uintptr_t)bufq, msg)

/**
 * @brief Detects when the buffer queue is empty.
 *
 * This function is expected to be called from within critical sections, and is not threadsafe.
 */
#define ts_impl_bufq_empty(queue) ((queue)->first == NULL)

/**
 * Initialize the struct ts_impl_bufq.
 */
int ts_impl_bufq_init(struct ts_impl_bufq *queue)
{
    memset(queue, 0, sizeof(struct ts_impl_bufq));

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    int ret = pthread_mutex_init(&queue->one_big_mutex, &attr);
    if (ret != 0) {
        TS_IMPL_BUFQ_LOGD("pthread_mutex_init failed", queue);
        return ret;
    }

    ret = pthread_cond_init(&queue->not_empty, NULL);
    if (ret != 0) {
        TS_IMPL_BUFQ_LOGD("pthread_cond_init not_empty failed", queue);
        return ret;
    }

    /* Set all the data in the queue to NULL */
    queue->first = NULL;
    queue->empty_waiters = 0;

    return 0;
}

/**
 * @brief Last buffer in buffer queue.
 *
 * @return Pointer to last buffer or NULL.
 */
struct ts_impl_buf *ts_impl_bufq_last(struct ts_impl_bufq *bufq)
{
    struct ts_impl_buf *last = NULL;
    struct ts_impl_buf *next = bufq->first;

    while (next != NULL) {
        last = next;
        next = last->next;
    }

    return last;
}

/**
 * Push new buffer onto the queue. If the queue is full, return RPA_EAGAIN. If
 * the push operation completes successfully, it signals other threads
 * waiting in rpa_queue_pop() that they may continue consuming sockets.
 */
int ts_impl_bufq_put(struct ts_impl_bufq *queue, struct ts_impl_buf *buf)
{
    int ret = pthread_mutex_lock(&queue->one_big_mutex);
    if (ret != 0) {
        return ret;
    }

    /* Put buffer as last element into queue */
    if (queue->first == NULL) {
        queue->first = buf;
    }
    else {
        struct ts_impl_buf *last = ts_impl_bufq_last(queue);
        last->next = buf;
    }
    buf->next = NULL;

    /* Signal not empty */
    if (queue->empty_waiters) {
        TS_IMPL_BUFQ_LOGD("sig !empty", queue);
        ret = pthread_cond_signal(&queue->not_empty);
        if (ret != 0) {
            pthread_mutex_unlock(&queue->one_big_mutex);
            return ret;
        }
    }

    pthread_mutex_unlock(&queue->one_big_mutex);
    return 0;
}

int ts_impl_bufq_get(struct ts_impl_bufq *queue, thingset_time_ms_t timeout_ms,
                     struct ts_impl_buf **buf)
{
    int ret = pthread_mutex_lock(&queue->one_big_mutex);
    if (ret != 0) {
        return ret;
    }

    /* Keep waiting until we wake up and find that the queue is not empty. */
    if (ts_impl_bufq_empty(queue)) {
        if (timeout_ms == THINGSET_TIMEOUT_IMMEDIATE) {
            return -EAGAIN;
        }

        queue->empty_waiters++;
        if (timeout_ms == THINGSET_TIMEOUT_FOREVER) {
            ret = pthread_cond_wait(&queue->not_empty, &queue->one_big_mutex);
        } else {
            struct timespec abstime = thingset_time_timeout_spec(timeout_ms);
            ret = pthread_cond_timedwait(&queue->not_empty, &queue->one_big_mutex, &abstime);
        }

        queue->empty_waiters--;
        if (ret != 0) {
            pthread_mutex_unlock(&queue->one_big_mutex);
            return ret;
        }

        /* If we wake up and it's still empty, then we were interrupted */
        if (ts_impl_bufq_empty(queue)) {
            TS_IMPL_BUFQ_LOGD("queue empty (intr)", queue);
            ret = pthread_mutex_unlock(&queue->one_big_mutex);
            if (ret == 0) {
                ret = -EINTR;
            }
            return ret;
        }
    }

    /* Get buffer from first element in queue */
    *buf = queue->first;
    queue->first = queue->first->next;

    pthread_mutex_unlock(&queue->one_big_mutex);
    return 0;
}

int ts_impl_bufq_is_empty(struct ts_impl_bufq *bufq)
{
    int ret = pthread_mutex_lock(&bufq->one_big_mutex);
    if (ret != 0) {
        return ret;
    }

    ret = ts_impl_bufq_empty(bufq) ? -ENOTEMPTY : 0;

    pthread_mutex_unlock(&bufq->one_big_mutex);
    return ret;
}
