/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Native implementation of ThingSet logging.
 */

#ifndef TS_IMPL_LOG_H_
#define TS_IMPL_LOG_H_

#if TS_CONFIG_LOG && TS_CONFIG_LOG_LEVEL > 0
#define TS_IMPL_LOGE(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define TS_IMPL_LOGE(...)
#endif

#if TS_CONFIG_LOG && TS_CONFIG_LOG_LEVEL > 1
#define TS_IMPL_LOGW(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define TS_IMPL_LOGW(...)
#endif

#if TS_CONFIG_LOG && TS_CONFIG_LOG_LEVEL > 2
#define TS_IMPL_LOGI(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define TS_IMPL_LOGI(...)
#endif

#if TS_CONFIG_LOG && TS_CONFIG_LOG_LEVEL > 3
#define TS_IMPL_LOGD(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#else
#define TS_IMPL_LOGD(...)
#endif


#include <inttypes.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* None */

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_LOG_H_ */
