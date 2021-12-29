/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet memory blocks (private interface)
 */

#ifndef TS_MEM_H_
#define TS_MEM_H_

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
 * @brief Implementation for @ref ts_mem.
 */
#define ts_impl_mem

/**
 * @brief Implementation for @ref TS_MEM_DEFINE.
 */
#define TS_IMPL_MEM_DEFINE(name, mem_size)

/**
 * @brief Implementation for @ref TS_MEM_DECLARE.
 */
#define TS_IMPL_MEM_DECLARE(name)

#endif /* TS_DOXYGEN */

/**
 * @brief Implementation for @ref ts_mem_alloc.
 */
int ts_impl_mem_alloc(struct ts_impl_mem *mem_pool, size_t block_size, thingset_time_ms_t timeout_ms,
                      void **mem_block);

/**
 * @brief Implementation for @ref ts_mem_free.
 */
int ts_impl_mem_free(struct ts_impl_mem *mem_pool, const void *mem_block);

/**
 * @}  <!-- ts_impl_api -->
 */

/**
 * @brief ThingSet memory blocks.
 *
 * Memory blocks can be dynamically allocated from a designated memory pool.
 *
 * A memory pool can be defined and initialized at compile time by calling @ref TS_MEM_DEFINE.
 *
 * @defgroup ts_mem_api_priv ThingSet memory blocks (private interface)
 * @{
 */

/**
 * @def TS_MEM_DEFINE
 *
 * @brief Statically define and initialize a memory pool in a public (non-static) scope.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] name Name of the memory pool.
 * @param[in] mem_size Size of memory region (in bytes).
 */
#define TS_MEM_DEFINE(name, mem_size) TS_IMPL_MEM_DEFINE(name, mem_size)

/**
 * @def TS_MEM_DECLARE
 *
 * @brief Declare a memory pool in a public (non-static) scope.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] name Name of the memory pool.
 */
#define TS_MEM_DECLARE(name) TS_IMPL_MEM_DECLARE(name)

/**
 * @brief ThingSet memory pool structure name.
 *
 * @note To be provided by the implementation.
 */
#define ts_mem ts_impl_mem

/**
 * @brief Allocate memory block from a memory pool.
 *
 * Allocates and returns a memory block from the memory region owned by the memory pool. If no
 * memory is available immediately, the call will block for the specified timeout (constructed via
 * the standard timeout API, or TS_NO_WAIT or TS_FOREVER) waiting for memory to be freed. If the
 * allocation cannot be performed by the expiration of the timeout, an error will be returned.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] mem_pool Address of the memory pool.
 * @param[in] block_size Size of memory block (in bytes).
 * @param[in] timeout_ms Maximum time to wait in milliseconds.
 * @param[out] mem_block Pointer to memory block.
 * @return 0 on success, <0 otherwise.
 * @retval -ENOMEM Returned without waiting on no memory available.
 * @retval -EAGAIN Waiting period timed out.
 * @retval -EINVAL Invalid data supplied.
 */
static inline int ts_mem_alloc(struct ts_mem *mem_pool, size_t block_size,
                               thingset_time_ms_t timeout_ms, void **mem_block)
{
    return ts_impl_mem_alloc(mem_pool, block_size, timeout_ms, mem_block);
}

/**
 * @brief Free memory block allocated from a memory pool.
 *
 * Release a previously allocated memory block back to its associated memory pool.
 *
 * @note To be provided by the implementation.
 *
 * @param[in] mem_pool Address of the memory pool.
 * @param[in] mem_block Pointer to memory block.
 * @return 0 on success, <0 otherwise
 */
static inline int ts_mem_free(struct ts_mem *mem_pool, const void *mem_block)
{
    return ts_impl_mem_free(mem_pool, mem_block);
}

/**
 * @} <!-- ts_mem_api_priv -->
 */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TS_MEM_H_ */
