/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet communication buffer queue (private interface).
 */

#ifndef TS_BUFQ_H_
#define TS_BUFQ_H_

#include "thingset_env.h"
#include "thingset_time.h"

#include "ts_log.h"
#include "ts_buf.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ThingSet environment implementation interface */

/**
 * @addtogroup ts_impl_api
 * @{
 */

#if TS_DOXYGEN

/**
 * @brief Implementation for @ref TS_BUFQ_DEFINE.
 */
#define TS_IMPL_BUFQ_DEFINE(bufq_name)

/**
 * @brief Implementation for @ref TS_BUFQ_DECLARE.
 */
#define TS_IMPL_BUFQ_DECLARE(bufq_name)

/**
 * @brief Implementation for @ref ts_bufq.
 */
#define ts_impl_bufq

#endif /* TS_DOXYGEN */

/**
 * @brief Implementation for @ref ts_bufq_init.
 */
int ts_impl_bufq_init(struct ts_impl_bufq *bufq);

/**
 * @brief Implementation for @ref ts_bufq_put.
 */
int ts_impl_bufq_put(struct ts_impl_bufq *bufq, struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_bufq_get.
 */
int ts_impl_bufq_get(struct ts_impl_bufq *bufq, thingset_time_ms_t timeout_ms,
                     struct ts_impl_buf **buffer);

/**
 * @brief Implementation for @ref ts_bufq_is_empty.
 */
int ts_impl_bufq_is_empty(struct ts_impl_bufq *bufq);

/**
 * @}  <!-- ts_impl_api -->
 */

/**
 * @brief ThingSet communication buffer queue.
 *
 * @defgroup ts_bufq_api_priv ThingSet communication buffer queue (private interface)
 * @{
 */

/**
 * @def TS_BUFQ_DEFINE Statically define and initialize a buffer queue.
 *
 * @note TS_IMPL_BUFQ_DEFINE to be provided by the implementation.
 */
#define TS_BUFQ_DEFINE(bufq_name) TS_IMPL_BUFQ_DEFINE(bufq_name)

/**
 * @def TS_BUFQ_DECLARE Declare a buffer queue.
 *
 * @note TS_IMPL_BUFQ_DECLARE to be provided by the implementation.
 */
#define TS_BUFQ_DECLARE(bufq_name) TS_IMPL_BUFQ_DECLARE(bufq_name)

/**
 * @brief ThingSet communication buffer queue structure name.
 *
 * @note To be provided by the implementation.
 */
#define ts_bufq ts_impl_bufq

/**
 * @brief Initialize a buffer queue.
 *
 * @note ts_impl_bufq_init to be provided by the implementation.
 *
 * @return 0 on success, < 0 otherwise.
 */
static inline int ts_bufq_init(struct ts_bufq *bufq)
{
    int ret = ts_impl_bufq_init(bufq);
    TS_LOGD("BUFQ: %s initialized queue 0x%" PRIXPTR " returning %d", __func__, (uintptr_t)bufq,
            ret);
    return ret;
}

/**
 * @brief Add a buffer to the queue.
 *
 * @note ts_impl_bufq_put to be provided by the implementation.
 *
 * @return 0 on success, < 0 otherwise.
 */
static inline int ts_bufq_put(struct ts_bufq *bufq, struct ts_buf *buffer)
{
    int ret = ts_impl_bufq_put(bufq, buffer);
    TS_LOGD("BUFQ: %s put buffer 0x%" PRIXPTR " to queue 0x%" PRIXPTR " returning %d", __func__,
            (uintptr_t)buffer, (uintptr_t)bufq, ret);
    return ret;
}

/**
 * @brief Get a buffer from the queue.
 *
 * @note ts_impl_bufq_get to be provided by the implementation.
 *
 * @return 0 on success, < 0 otherwise.
 */
static inline int ts_bufq_get(struct ts_bufq *bufq, thingset_time_ms_t timeout_ms, struct ts_buf **buffer)
{
    int ret = ts_impl_bufq_get(bufq, timeout_ms, buffer);
    TS_LOGD("BUFQ: %s got buffer 0x%" PRIXPTR " from queue 0x%" PRIXPTR " returning %d", __func__,
            (uintptr_t)*buffer, (uintptr_t)bufq, ret);
    return ret;
}

/**
 * @brief Query a queue to see if it has buffers available.
 *
 * @note ts_impl_bufq_is_empty to be provided by the implementation.
 *
 * @return Non-zero if the queue is empty, 0 if buffers are available.
 */
static inline int ts_bufq_is_empty(struct ts_bufq *bufq)
{
    int ret = ts_impl_bufq_is_empty(bufq);
    TS_LOGD("BUFQ: %s on queue 0x%" PRIXPTR " returning %d", __func__, (uintptr_t)bufq, ret);
    return ret;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_bufq_api_priv -->
 */

#endif /* TS_BUFQ_H_ */
