/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Native implementation of ThingSet memory blocks.
 */

#ifndef TS_IMPL_MEM_H_
#define TS_IMPL_MEM_H_

#include <stdbool.h>
#include <stdint.h>
#include <malloc.h>

#if defined(__cplusplus) && __cplusplus <= 201703L
#ifndef _Atomic
#define _Atomic
#endif
#endif
#include <stdatomic.h>

#include "../../src/thingset_time.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ts_impl_mem_block {
    size_t size;
    uint8_t data[];
};

struct ts_impl_mem {
    size_t size;
    atomic_size_t alloc;
};

#define TS_IMPL_MEM_DEFINE(name, mem_size) struct ts_impl_mem name = {                  \
        .size = mem_size,                                                               \
        .alloc = 0                                                                      \
    }

#define TS_IMPL_MEM_DECLARE(name) extern struct ts_impl_mem name

static inline int ts_impl_mem_alloc(struct ts_impl_mem *mem_pool, size_t block_size,
                                    thingset_time_ms_t timeout_ms, void **mem_block)
{
    /* Allocate block_size block */
    size_t alloc = atomic_fetch_add(&mem_pool->alloc, block_size);
    if ((mem_pool->size - alloc) < block_size) {
        /* We do not have enough space */
        (void)atomic_fetch_sub(&mem_pool->alloc, block_size);
        return -ENOMEM;
    }

    struct ts_impl_mem_block *block =
        (struct ts_impl_mem_block *)malloc(sizeof(struct ts_impl_mem_block) + block_size);
    if (block == NULL) {
        /* We do not have enough space */
        (void)atomic_fetch_sub(&mem_pool->alloc, block_size);
        return -ENOMEM;
    }
    block->size = block_size;
    *mem_block = &block->data;
    return 0;
}

static inline int ts_impl_mem_free(struct ts_impl_mem *mem_pool, const void *mem_block)
{
    if (mem_block == 0) {
        return 0;
    }

    /*
     * Cast away const from block.
     * This is acceptable here as const defines the potential usage of the block for pseudo
     * constant data - but not the characteristic of the block. Blocks are always mutable.
     */
    union {
        const void *pseudo_immutable_block;
        void *mutable_block;
    } blk = { .pseudo_immutable_block = mem_block};

    struct ts_impl_mem_block *block = (struct ts_impl_mem_block *)((uint8_t *)blk.mutable_block -
                                                        offsetof(struct ts_impl_mem_block, data));
    (void)atomic_fetch_sub(&mem_pool->alloc, block->size);
    free(block);

    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_MEM_H_ */
