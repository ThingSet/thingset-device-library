/*
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Native implementation of ThingSet endian conversion.
 */

#ifndef TS_IMPL_ENDIAN_H_
#define TS_IMPL_ENDIAN_H_

#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t ts_impl_endian_get_be16(const uint8_t src[2])
{
    return ntohs(*(uint16_t *)src);
}

static inline uint32_t ts_impl_endian_get_be32(const uint8_t src[4])
{
    return ntohl(*(uint32_t *)src);
}

static inline void ts_impl_endian_put_be16(uint16_t val, uint8_t dst[2])
{
    *(uint16_t *)dst = htons(val);
}

static inline void ts_impl_endian_put_be32(uint32_t val, uint8_t dst[4])
{
    *(uint32_t *)dst = htonl(val);
}

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_ENDIAN_H_ */
