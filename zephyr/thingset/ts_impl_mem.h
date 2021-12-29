/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet memory blocks.
 */

#ifndef TS_IMPL_MEM_H_
#define TS_IMPL_MEM_H_

#include "../../src/thingset_time.h"

#include <kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ts_impl_mem k_heap

#define TS_IMPL_MEM_DEFINE(name, mem_size) K_HEAP_DEFINE(name, Z_HEAP_MIN_SIZE + mem_size)

#define TS_IMPL_MEM_DECLARE(name) extern struct ts_impl_mem name

static inline int ts_impl_mem_alloc(struct ts_impl_mem *mem_pool, size_t block_size,
                                    thingset_time_ms_t timeout_ms, void **mem_block)
{
    *mem_block = k_heap_aligned_alloc(mem_pool, 4, block_size,
                                      TS_IMPL_ZEPHYR_TIMEOUT_MS(timeout_ms));
    return (*mem_block == NULL) ? -ENOMEM : 0;
}

static inline int ts_impl_mem_free(struct ts_impl_mem *mem_pool, const void *mem_block)
{
    /*
     * Cast away const from block.
     * This is acceptable here as const defines the potential usage of the block for pseudo
     * constant data - but not the characteristic of the block. Blocks are always mutable.
     */
    union {
        const void *pseudo_immutable_block;
        void *mutable_block;
    } block = { .pseudo_immutable_block = mem_block};

    k_heap_free(mem_pool, block.mutable_block);
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_MEM_H_ */
