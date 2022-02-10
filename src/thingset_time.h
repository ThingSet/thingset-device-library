/*
 * Copyright (c) 2021..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet time support (public interface).
 */

#ifndef THINGSET_TIME_H_
#define THINGSET_TIME_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ThingSet time support.
 *
 * Thingset time support is based on posix time.
 *
 * @defgroup ts_time_api_pub ThingSet time support (public interface)
 * @{
 */

/**
 * @brief Mark infinite timeout delay,
 *
 * This macro generates a timeout delay that instructs a ThingSet API functions to wait as long
 * as necessary to perform the requested operation.
 */
#define THINGSET_TIMEOUT_FOREVER UINT32_MAX

/**
 * @brief Mark immediate timeout delay.
 *
 * This macro generates a timeout delay that instructs a ThingSet API functions to not wait to
 * perform the requested operation.
 */
#define THINGSET_TIMEOUT_IMMEDIATE 0

/**
 * @brief ThingSet maximum system time value in milliseconds.
 *
 * This is the last value before roll over.
 */
#define THINGSET_TIME_MS_MAX UINT32_MAX

/**
 * @brief Type of ThingSet system time in milliseconds.
 */
typedef uint32_t thingset_time_ms_t;

/**
 * @brief Get system time in milliseconds.
 *
 * @return system time in milliseconds
 */
thingset_time_ms_t thingset_time_ms(void);

/**
 * @brief Get elapsed system time in milliseconds.
 *
 * @param[in] reftime Reference time in milliseconds.
 * @return delta in milliseconds
 */
thingset_time_ms_t thingset_time_ms_delta(thingset_time_ms_t reftime);

/**
 * @brief Absolute timeout time as posix timespec.
 *
 * @param[in] timeout_ms Timeout in milliseconds.
 * @return Absolute timeout time as posix timespec.
 */
struct timespec thingset_time_timeout_spec(thingset_time_ms_t timeout_ms);

#ifdef __cplusplus
} /* extern "C" */
#endif

/**
 * @} <!-- ts_time_api_pub -->
 */

#endif /* THINGSET_TIME_H_ */
