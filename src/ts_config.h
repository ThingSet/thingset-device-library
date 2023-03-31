/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef TS_CONFIG_H_
#define TS_CONFIG_H_

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
 * This should be disabled for most 8-bit microcontrollers to increase
 * performance
 */
#if !defined(TS_64BIT_TYPES_SUPPORT) && !defined(CONFIG_THINGSET_64BIT_TYPES_SUPPORT)
#define TS_64BIT_TYPES_SUPPORT 0 // default: no support
#elif !defined(TS_64BIT_TYPES_SUPPORT)
#define TS_64BIT_TYPES_SUPPORT CONFIG_THINGSET_64BIT_TYPES_SUPPORT
#endif

/*
 * Switch on support for CBOR decimal fraction data type which stores a decimal mantissa
 * and a constant decimal exponent. This allows to use e.g. millivolts internally instead
 * of floating point numbers, while still communicating the SI base unit (volts).
 */
#if !defined(TS_DECFRAC_TYPE_SUPPORT) && !defined(CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT)
#define TS_DECFRAC_TYPE_SUPPORT 0 // default: no support
#elif !defined(TS_DECFRAC_TYPE_SUPPORT)
#define TS_DECFRAC_TYPE_SUPPORT CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
#endif

/*
 * Switch on support for CBOR byte strings, which can store any sort of binary data and
 * can be used e.g. for firmware upgrades. Byte strings are not supported by JSON.
 */
#if !defined(TS_BYTE_STRING_TYPE_SUPPORT) && !defined(CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT)
#define TS_BYTE_STRING_TYPE_SUPPORT 0 // default: no support
#elif !defined(TS_BYTE_STRING_TYPE_SUPPORT)
#define TS_BYTE_STRING_TYPE_SUPPORT CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT
#endif

/*
 * The ThingSet specification v0.5 introduces a different data layout compared to previous
 * versions where the data is grouped by entities of the device (like battery, actuator)
 * instead of the data type (e.g. measurement, configuration). The data type is described by
 * a single character prefix in the item name.
 * The new grouping allows to have same item names in different groups, so for unambigous
 * description of the data the nested structure has to be maintained in statements.
 *
 * With nested JSON enabled, requesting names for IDs will also return the entire path (as
 * a JSON pointer) instead of just the data item name.
 *
 * This option is introduced to maintain compatibility with legacy firmware and will be
 * enabled by default in the future.
 */
#if !defined(TS_NESTED_JSON) && !defined(CONFIG_THINGSET_NESTED_JSON)
#define TS_NESTED_JSON 0 // default: no nested JSON for legacy code
#elif !defined(TS_NESTED_JSON)
#define TS_NESTED_JSON CONFIG_THINGSET_NESTED_JSON
#endif

#endif /* TS_CONFIG_H_ */
