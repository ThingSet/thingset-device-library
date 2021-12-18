/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet communication buffers (private interface)
 */

#ifndef TS_BUF_H_
#define TS_BUF_H_

#include "thingset_env.h"
#include "thingset_time.h"

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
 * @brief Implementation for @ref ts_buf.
 */
#define ts_impl_buf

#endif /* TS_DOXYGEN */

/**
 * @brief Implementation for @ref ts_buf_alloc.
 */
int ts_impl_buf_alloc(uint16_t size, thingset_time_ms_t timeout_ms, struct ts_impl_buf **buffer);

/**
 * @brief Implementation for @ref ts_buf_alloc_with_data.
 */
int ts_impl_buf_alloc_with_data(void *data, uint16_t size, thingset_time_ms_t timeout_ms,
                                struct ts_impl_buf **buffer);

/**
 * @brief Implementation for @ref ts_buf_unref.
 */
int ts_impl_buf_unref(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_ref.
 */
int ts_impl_buf_ref(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_size.
 */
uint16_t ts_impl_buf_size(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_len.
 */
uint16_t ts_impl_buf_len(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_headroom.
 */
uint16_t ts_impl_buf_headroom(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_tailroom.
 */
uint16_t ts_impl_buf_tailroom(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_data.
 */
uint8_t *ts_impl_buf_data(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_unref.
 */
uint8_t *ts_impl_buf_tail(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_reserve.
 */
void ts_impl_buf_reserve(struct ts_impl_buf *buffer, uint16_t reserve);

/**
 * @brief Implementation for @ref ts_buf_reset.
 */
void ts_impl_buf_reset(struct ts_impl_buf *buffer);

/**
 * @brief Implementation for @ref ts_buf_clone.
 */
int ts_impl_buf_clone(struct ts_impl_buf *buffer, thingset_time_ms_t timeout_ms, struct ts_impl_buf **clone);

/**
 * @brief Implementation for @ref ts_buf_add.
 */
uint8_t *ts_impl_buf_add(struct ts_impl_buf *buffer, uint16_t len);

/**
 * @brief Implementation for @ref ts_buf_add_mem.
 */
uint8_t *ts_impl_buf_add_mem(struct ts_impl_buf *buffer, const uint8_t *data, uint16_t len);

/**
 * @brief Implementation for @ref ts_buf_add_u8.
 */
uint8_t *ts_impl_buf_add_u8(struct ts_impl_buf *buffer, uint8_t val);

/**
 * @brief Implementation for @ref ts_buf_remove.
 */
uint8_t *ts_impl_buf_remove(struct ts_impl_buf *buffer, uint16_t len);

/**
 * @brief Implementation for @ref ts_buf_pull.
 */
uint8_t *ts_impl_buf_push(struct ts_impl_buf *buffer, uint16_t len);

/**
 * @brief Implementation for @ref ts_buf_unref.
 */
uint8_t *ts_impl_buf_pull(struct ts_impl_buf *buffer, uint16_t len);

/**
 * @brief Implementation for @ref ts_buf_free_all.
 */
int ts_impl_buf_free_all(void);

/**
 * @}  <!-- ts_impl_api -->
 */

/**
 * @brief ThingSet communication buffers.
 *
 * Buffers are divided into three buffer parts:
 * - headroom
 * - data
 * - tailroom
 *
 * All of the parts may be of zero size.
 *
 * @defgroup ts_buf_api_priv ThingSet communication buffers (private interface)
 * @{
 */

/**
 * @brief ThingSet communication buffer structure name.
 *
 * @note To be provided by the implementation.
 */
#define ts_buf ts_impl_buf

/**
 * @brief Allocate a ThingSet communication buffer from the buffer pool.
 *
 * The communication buffer is allocated with reference count set to 1.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] size The size of the buffer
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @param[out] buffer Pointer to buffer
 * @return 0 on success, <0 otherwise
 */
static inline int ts_buf_alloc(uint16_t size, thingset_time_ms_t timeout_ms, struct ts_buf **buffer)
{
    return ts_impl_buf_alloc(size, timeout_ms, buffer);
}


/**
 * @brief Allocate a ThingSet communication buffer from the buffer pool with external data pointer.
 *
 * Allocate a new communication buffer from a pool, where the data pointer comes from the user and
 * not from the pool.
 *
 * The communication buffer is allocated with reference count set to 1.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] data Pointer to external data.
 * @param[in] size The size of the buffer.
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @param[out] buffer Pointer to buffer.
 * @return 0 on success, <0 otherwise
 */
static inline int ts_buf_alloc_with_data(void *data, uint16_t size, thingset_time_ms_t timeout_ms,
                                         struct ts_buf **buffer)
{
    return ts_impl_buf_alloc_with_data(data, size, timeout_ms, buffer);
}

/**
 * @brief Mark ThingSet communication buffer unused.
 *
 * Decrement the reference count of a buffer. The buffer is put back into the
 * pool if the reference count reaches zero.
 *
 * @note The buffer shall not be accessed after it is marked unused.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return 0 on success, <0 otherwise
 */
static inline int ts_buf_unref(struct ts_buf *buffer)
{
    return ts_impl_buf_unref(buffer);
}

/**
 * @brief Mark ThingSet communication buffer used.
 *
 * Increment the reference count of a buffer.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return 0 on success, <0 otherwise
 */
static inline int ts_buf_ref(struct ts_buf *buffer)
{
    return ts_impl_buf_ref(buffer);
}

/**
 * @brief Amount of data that this ThingSet communication buffer can store.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return Amount of data that this buffer can store.
 */
static inline uint16_t ts_buf_size(struct ts_buf *buffer)
{
    return ts_impl_buf_size(buffer);
}

/**
 * @brief Amount of data that is stored in this ThingSet communication buffer.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return Amount of data that is stored in this buffer.
 */
static inline uint16_t ts_buf_len(struct ts_buf *buffer)
{
    return ts_impl_buf_len(buffer);
}

/**
 * @brief Size of buffer headroom.
 *
 * Free space at the beginning of the buffer.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return Number of bytes available at the beginning of the buffer.
 */
static inline uint16_t ts_buf_headroom(struct ts_buf *buffer)
{
    return ts_impl_buf_headroom(buffer);
}

/**
 * @brief Size of buffer tailroom.
 *
 * Free space at the end of the buffer.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return Number of bytes available at the end of the buffer.
 */
static inline uint16_t ts_buf_tailroom(struct ts_buf *buffer)
{
    return ts_impl_buf_tailroom(buffer);
}

/**
 * @brief Get the data pointer for a ThingSet communication buffer.
 *
 * Data pointer points to the first data stored in buffer.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return data pointer of buffer.
 */
static inline uint8_t *ts_buf_data(struct ts_buf *buffer)
{
    return ts_impl_buf_data(buffer);
}

/**
 * @brief Get the tail pointer for a ThingSet communication buffer.
 *
 * Tail pointer points after the last data stored in buffer.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return tail pointer of buffer.
 */
static inline uint8_t *ts_buf_tail(struct ts_buf *buffer)
{
    return ts_impl_buf_tail(buffer);
}

/**
 * @brief Initialize buffer with the given headroom.
 *
 * @note The buffer is not expected to contain any data when this API is called.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @return tail pointer of buffer.
 */
static inline void ts_buf_reserve(struct ts_buf *buffer, uint16_t reserve)
{
    ts_impl_buf_reserve(buffer, reserve);
}

/**
 * @brief Reset buffer.
 *
 * Reset buffer data so the buffer can be reused for other purposes.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 */
static inline void ts_buf_reset(struct ts_buf *buffer)
{
    ts_impl_buf_reset(buffer);
}

/**
 * @brief Clone buffer.
 *
 * Duplicate given buffer including any data currently stored.
 *
 * @param[in] buffer Pointer to the source buffer.
 * @param[in] timeout_ms maximum time to wait in milliseconds.
 * @param[out] clone Pointer to the cloned buffer.
 * @return The original tail of the destinatio buffer, before incremented by new data.
 */
static inline int ts_buf_clone(struct ts_buf *buffer, thingset_time_ms_t timeout_ms,
                               struct ts_buf **clone)
{
    return ts_impl_buf_clone(buffer, timeout_ms, clone);
}

/**
 * @brief Prepare data to be added at the end of the buffer.
 *
 * Increments the data length of a buffer to account for more data at the end.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @param[in] len Number of bytes to increment the length with.
 * @return The original tail of the buffer, before incremented by len.
 */
static inline uint8_t *ts_buf_add(struct ts_buf *buffer, uint16_t len)
{
    return ts_impl_buf_add(buffer, len);
}

/**
 * @brief Copy the given number of bytes to the end of the buffer.
 *
 * Increments the data length of the buffer to account for more data at the end.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @param[in] data Location of data to be added.
 * @param[in] len Number of bytes to by added.
 * @return The original tail of the buffer, before incremented by len.
 */
static inline uint8_t *ts_buf_add_mem(struct ts_buf *buffer, const uint8_t *data, uint16_t len)
{
    return ts_impl_buf_add_mem(buffer, data, len);
}

/**
 * @brief Add (8-bit) byte at the end of the buffer.
 *
 * Increments the data length of the buffer to account for more data at the end.
 *
 * @note ts_impl_buf_add_u8 to be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @param[in] val Byte value to be added.
 * @return Pointer to the value added.
 */
static inline uint8_t *ts_buf_add_u8(struct ts_buf *buffer, uint8_t val)
{
    return ts_impl_buf_add_u8(buffer, val);
}

/**
 * @brief Remove data from the end of the buffer.
 *
 * Removes data from the end of the buffer by modifying the buffer length (not the size).
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @param[in] len Number of bytes to remove.
 * @return The new tail of the buffer.
 */
static inline uint8_t *ts_buf_remove(struct ts_buf *buffer, uint16_t len)
{
    return ts_impl_buf_remove(buffer, len);
}

/**
 * @brief Prepare data to be added at start of the buffer.
 *
 * Modifies the data pointer and buffer length to account for more data in the beginning of the
 * buffer.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @param[in] len Number of bytes to add to the beginning.
 * @return The new beginning of the buffer data.
 */
static inline uint8_t *ts_buf_push(struct ts_buf *buffer, uint16_t len)
{
    return ts_impl_buf_push(buffer, len);
}

/**
 * @brief Remove data from the beginning of the buffer.
 *
 * Removes data from the beginning of the buffer by modifying the data pointer and buffer length.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] buffer Pointer to the buffer.
 * @param[in] len Number of bytes to remove.
 * @return New beginning of the buffer data.
 */
static inline uint8_t *ts_buf_pull(struct ts_buf *buffer, uint16_t len)
{
    return ts_impl_buf_pull(buffer, len);
}

/**
 * @brief Mark all ThingSet communication buffers unused.
 *
 * Set reference count of all buffers to zero. Put back all buffers into the pool.
 *
 * @note Any buffers shall not be accessed after this function was executed.
 *
 * @note To be provided by the implementation.
 *
 * @return 0 on success, <0 otherwise
 */
static inline int ts_buf_free_all(void)
{
    return ts_impl_buf_free_all();
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_buf_api_priv -->
 */

#endif /* TS_BUF_H_ */
