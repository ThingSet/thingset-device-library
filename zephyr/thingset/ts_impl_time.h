/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Zephyr implementation of ThingSet time support.
 */

#ifndef TS_IMPL_TIME_H_
#define TS_IMPL_TIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#define TS_IMPL_ZEPHYR_TIMEOUT_MS(timeout_ms) \
    (timeout_ms == THINGSET_TIMEOUT_FOREVER ? K_FOREVER : K_MSEC(timeout_ms))

#ifdef __cplusplus
}
#endif

#endif /* TS_IMPL_TIME_H_ */
