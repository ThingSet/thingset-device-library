/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Native implementation of ThingSet communication buffers.
 */

#ifndef TS_IMPL_BUF_H_
#define TS_IMPL_BUF_H_

#include <stdbool.h>
#include <stdint.h>

#include "../../src/thingset_time.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Flag indicating that the buffer's associated data pointer, points to
 *        externally allocated memory.
 *
 * Therefore once ref goes down to zero, the
 * pointed data will not need to be deallocated. This never needs to be
 * explicitly set or unset by the net_buf API user. Such net_buf is
 * exclusively instantiated via net_buf_alloc_with_data() function.
 * Reference count mechanism however will behave the same way, and ref
 * count going to 0 will free the net_buf but not the data pointer in it.
 */
#define TS_IMPL_BUF_EXTERNAL_DATA  (0x02U)

/**
 * @brief Native communication buffer representation.
 */
struct ts_impl_buf {
    /** @brief Next buffer pointer to be used for buffer queues */
    struct ts_impl_buf *next;

    /** @brief Reference count. */
    atomic_int_least8_t ref;

    /** @brief Bit-field of buffer flags. */
    uint8_t flags;

    /** @brief Pointer to the start of data in the buffer. */
    uint8_t *data;

    /**
     * @brief Length of the data behind the data pointer.
     *
     * To determine the max length, use ts_impl_buf_max_len(), not #size!
     */
    uint16_t len;

    /** @brief Amount of data that ts_impl_buf#__buf can store. */
    uint16_t size;

    /**
     * @brief Start of the data storage. Not to be accessed directly
     *         (the data pointer should be used instead).
     */
    uint8_t *__buf;
};

int ts_impl_buf_alloc(uint16_t size, thingset_time_ms_t timeout_ms, struct ts_impl_buf **buffer);

int ts_impl_buf_alloc_with_data(void *data, uint16_t size, thingset_time_ms_t timeout_ms,
                                struct ts_impl_buf **buffer);

int ts_impl_buf_unref(struct ts_impl_buf *buffer);

int ts_impl_buf_ref(struct ts_impl_buf *buffer);

static inline uint16_t ts_impl_buf_size(struct ts_impl_buf *buffer)
{
    return buffer->size;
}

static inline uint16_t ts_impl_buf_len(struct ts_impl_buf *buffer)
{
    return buffer->len;
}

static uint16_t ts_impl_buf_headroom(struct ts_impl_buf *buffer)
{
	return buffer->data - buffer->__buf;
}

static inline uint16_t ts_impl_buf_tailroom(struct ts_impl_buf *buffer)
{
	return buffer->size - ts_impl_buf_headroom(buffer) - buffer->len;
}

static inline uint8_t *ts_impl_buf_data(struct ts_impl_buf *buffer)
{
    return buffer->data;
}

uint8_t *ts_impl_buf_tail(struct ts_impl_buf *buffer);

void ts_impl_buf_reserve(struct ts_impl_buf *buffer, uint16_t reserve);

void ts_impl_buf_reset(struct ts_impl_buf *buffer);

int ts_impl_buf_clone(struct ts_impl_buf *buffer, thingset_time_ms_t timeout_ms,
                      struct ts_impl_buf **clone);

uint8_t *ts_impl_buf_add(struct ts_impl_buf *buffer, uint16_t len);

uint8_t *ts_impl_buf_add_mem(struct ts_impl_buf *buffer, const uint8_t *data, uint16_t len);

uint8_t *ts_impl_buf_add_u8(struct ts_impl_buf *buffer, uint8_t val);

uint8_t *ts_impl_buf_remove(struct ts_impl_buf *buffer, uint16_t len);

uint8_t *ts_impl_buf_push(struct ts_impl_buf *buffer, uint16_t len);

uint8_t *ts_impl_buf_pull(struct ts_impl_buf *buffer, uint16_t len);

int ts_impl_buf_free_all(void);

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_BUF_H_ */
