/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet endian conversion (private interface)
 */

#ifndef TS_ENDIAN_H_
#define TS_ENDIAN_H_

/**
 * @brief ThingSet endian conversion.
 *
 * @defgroup ts_endian_api_priv ThingSet endian conversion (private interface)
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @} <!-- ts_endian_api_priv -->
 * @addtogroup ts_impl_api
 * @{
 */

/**
 * @brief Implementation for @ref ts_endian_get_be16.
 */
uint16_t ts_impl_endian_get_be16(const uint8_t src[2]);

/**
 * @brief Implementation for @ref ts_endian_get_be32.
 */
uint32_t ts_impl_endian_get_be32(const uint8_t src[4]);

/**
 * @brief Implementation for @ref ts_endian_put_be16.
 */
void ts_impl_endian_put_be16(uint16_t val, uint8_t dst[2]);

/**
 * @brief Implementation for @ref ts_endian_put_be32.
 */
void ts_impl_endian_put_be32(uint32_t val, uint8_t dst[4]);

/**
 * @}  <!-- ts_impl_api -->
 * @addtogroup ts_endian_api_priv
 * @{
 */


/**
 * @brief Get a 16-bit integer stored in big-endian format.
 *
 * Get a 16-bit integer, stored in big-endian format in a potentially unaligned memory location,
 * and convert it to the host endianness.
 *
 * @note ts_impl_endian_get_be16() to be provided by the implementation
 *
 * @param[in] src Pointer to the big-endian 16-bit integer to get.
 * @return 16-bit integer in host endianness.
 */
static inline uint16_t ts_endian_get_be16(const uint8_t src[2])
{
    return ts_impl_endian_get_be16(src);
}

/**
 * @brief Get a 32-bit integer stored in big-endian format.
 *
 * Get a 32-bit integer, stored in big-endian format in a potentially unaligned memory location,
 * and convert it to the host endianness.
 *
 * @note ts_impl_endian_get_be32() to be provided by the implementation
 *
 * @param[in] src Pointer to the big-endian 32-bit integer to get.
 * @return 16-bit integer in host endianness.
 */
static inline uint32_t ts_endian_get_be32(const uint8_t src[4])
{
    return ts_impl_endian_get_be32(src);
}

/**
 * @brief Put a 16-bit integer as big-endian to arbitrary location.
 *
 * Put a 16-bit integer, originally in host endianness, to a potentially unaligned memory location
 * in big-endian format.
 *
 * @note ts_impl_endian_put_be16() to be provided by the implementation
 *
 * @param[in] val 16-bit integer in host endianness.
 * @oaram[out] dst Destination memory address to store the result.
 */
static inline void ts_endian_put_be16(uint16_t val, uint8_t dst[2])
{
    ts_impl_endian_put_be16(val, dst);
}

/**
 * @brief Put a 32-bit integer as big-endian to arbitrary location.
 *
 * Put a 32-bit integer, originally in host endianness, to a potentially unaligned memory location
 * in big-endian format.
 *
 * @note ts_impl_endian_put_be32() to be provided by the implementation
 *
 * @param[in] val 32-bit integer in host endianness.
 * @oaram[out] dst Destination memory address to store the result.
 */
static inline void ts_endian_put_be32(uint32_t val, uint8_t dst[4])
{
    ts_impl_endian_put_be32(val, dst);
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_endian_api_priv -->
 */


#endif /* TS_ENDIAN_H_ */
