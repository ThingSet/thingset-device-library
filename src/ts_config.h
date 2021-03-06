/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#ifndef __TS_CONFIG_H_
#define __TS_CONFIG_H_

/*
 * Maximum number of expected JSON tokens (i.e. arrays, map keys, values,
 * primitives, etc.)
 *
 * Thingset throws an error if maximum number of tokens is reached in a
 * request or response.
 */
#ifndef TS_NUM_JSON_TOKENS
#define TS_NUM_JSON_TOKENS 50
#endif

/*
 * If verbose status messages are switched on, a response in text-based mode
 * contains not only the status code, but also a message.
 */
#ifndef TS_VERBOSE_STATUS_MESSAGES
#define TS_VERBOSE_STATUS_MESSAGES 1
#endif

/*
 * Switch on support for 64 bit variable types (uint64_t, int64_t, double)
 *
 * This should be commented out for most 8-bit microcontrollers to increase
 * performance
 */
#ifndef TS_64BIT_TYPES_SUPPORT
#define TS_64BIT_TYPES_SUPPORT 0        // default: no support
#endif

/*
 * Specify if the used lib-c implementation supports printing floats
 *
 * If float is not supported, a low-footprint custom implementation will be used for JSON output.
 *
 * Important: The custom implementation is not guaranteed to produce correct results under all
 * circumstances (e.g. very large or small numbers). If unsure, enable lib-c float support!
 */
#ifndef TS_PRINTF_FLOAT_SUPPORT
#ifdef CONFIG_NEWLIB_LIBC_FLOAT_PRINTF
#define TS_PRINTF_FLOAT_SUPPORT 1       // float support provided by Zephyr
#else
#define TS_PRINTF_FLOAT_SUPPORT 0       // use custom implementation
#endif
#endif

#endif /* __TS_CONFIG_H_ */
