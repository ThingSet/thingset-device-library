/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Native implementation of ThingSet communication buffer queue.
 */

#ifndef TS_IMPL_BUFQ_H_
#define TS_IMPL_BUFQ_H_

#include "ts_impl_buf.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ts_impl_bufq {
    struct ts_impl_buf *first;  /**< first buffer put into queue */
    uint32_t empty_waiters;
    pthread_mutex_t one_big_mutex;
    pthread_cond_t not_empty;
};

#define TS_IMPL_BUFQ_DEFINE(bufq_name) struct ts_impl_bufq bufq_name

#define TS_IMPL_BUFQ_DECLARE(bufq_name) extern struct ts_impl_bufq bufq_name

int ts_impl_bufq_init(struct ts_impl_bufq *bufq);
int ts_impl_bufq_put(struct ts_impl_bufq *bufq, struct ts_impl_buf *buffer);
int ts_impl_bufq_get(struct ts_impl_bufq *bufq, thingset_time_ms_t timeout_ms,
                     struct ts_impl_buf **buffer);
int ts_impl_bufq_is_empty(struct ts_impl_bufq *bufq);

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_BUFQ_H_ */
