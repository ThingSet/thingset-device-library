/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __TS_CONFIG_H_
#define __TS_CONFIG_H_

/*
 * Enable legacy C++ interface.
 *
 * This option enables the legacy C++ interface of the
 * ThingSet protocol library. Enable if your C++ code uses
 * DataNode or ArrayInfo instead of ThingSetDataNode or ThingSetArrayInfo.
 */
#if defined(__cplusplus) && !defined(CONFIG_THINGSET_CPP_LEGACY)
#define CONFIG_THINGSET_CPP_LEGACY 1
#endif

/*
 * Maximum number of expected JSON tokens (i.e. arrays, map keys, values,
 * primitives, etc.)
 *
 * Thingset throws an error if maximum number of tokens is reached in a
 * request or response.
 */
#if !defined(TS_NUM_JSON_TOKENS) && !defined(CONFIG_THINGSET_NUM_JSON_TOKENS)
#define TS_NUM_JSON_TOKENS 50
#elif !defined(TS_NUM_JSON_TOKENS)
#define TS_NUM_JSON_TOKENS CONFIG_THINGSET_NUM_JSON_TOKENS
#endif

/*
 * If verbose status messages are switched on, a response in text-based mode
 * contains not only the status code, but also a message.
 */
#if !defined(TS_VERBOSE_STATUS_MESSAGES) && !defined(CONFIG_THINGSET_VERBOSE_STATUS_MESSAGES)
#define TS_VERBOSE_STATUS_MESSAGES 1
#elif !defined(TS_VERBOSE_STATUS_MESSAGES)
#define TS_VERBOSE_STATUS_MESSAGES CONFIG_THINGSET_VERBOSE_STATUS_MESSAGES
#endif

/*
 * Switch on support for 64 bit variable types (uint64_t, int64_t, double)
 *
 * This should be disbaled for most 8-bit microcontrollers to increase
 * performance
 */
#if !defined(TS_64BIT_TYPES_SUPPORT) && !defined(CONFIG_THINGSET_64BIT_TYPES_SUPPORT)
#define TS_64BIT_TYPES_SUPPORT 0        // default: no support
#elif !defined(TS_64BIT_TYPES_SUPPORT)
#define TS_64BIT_TYPES_SUPPORT CONFIG_THINGSET_64BIT_TYPES_SUPPORT
#endif

#endif /* __TS_CONFIG_H_ */
