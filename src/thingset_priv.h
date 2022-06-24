/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_PRIV_H_
#define THINGSET_PRIV_H_

#include "thingset.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond INTERNAL_HIDDEN */

/* ThingSet adaptations to environment */
#if CONFIG_THINGSET_ZEPHYR

#include "../zephyr/thingset_zephyr.h"

#else /* ! CONFIG_THINGSET_ZEPHYR */

#define DEBUG 0

#define LOG_DBG(...) printf(__VA_ARGS__)
#define LOG_ERR(...) printf(__VA_ARGS__)
#define LOG_ALLOC_STR(str) str

#include <stdio.h>

#ifdef UNIT_TEST
#include <unity.h>
#endif

#endif /* CONFIG_THINGSET_ZEPHYR */

/** @endcond */

/**
 * Internal return type flags for payload data
 */
#define TS_RET_IDS              (1U << 0)       /**< Return type flag: IDs */
#define TS_RET_NAMES            (1U << 1)       /**< Return type flag: Names */
#define TS_RET_VALUES           (1U << 2)       /**< Return type flag: Values */
#define TS_RET_PATHS            (1U << 3)       /**< Return type flag: Paths */
#define TS_RET_DISCOVERY        (1U << 4)       /**< Return type flag: Discovery */

/**
 * Prepares JSMN parser, performs initial check of payload data and calls get/fetch/patch
 * functions.
 */
int ts_txt_process(struct ts_context *ts);

/**
 * Performs initial check of payload data and calls get/fetch/patch functions.
 */
int ts_bin_process(struct ts_context *ts);

/**
 * GET request (text mode).
 *
 * List child data objects (function called without content / parameters)
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the endpoint data object.
 * @param ret_type Return type flags (IDs, names and/or values).
 * @param element_number Element number extracted from path (only used for records).
 */
int ts_txt_get(struct ts_context *ts, const struct ts_data_object *endpoint, uint32_t ret_type,
               int element_number);

/**
 * GET request (binary mode).
 *
 * List child data objects (function called without content)
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the endpoint data object.
 * @param ret_type Return type flags (IDs, names and/or values).
 */
int ts_bin_get(struct ts_context *ts, const struct ts_data_object *endpoint, uint32_t ret_type);

/**
 * FETCH request (text mode).
 *
 * Read data object values (function called with an array as argument)
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the endpoint data object.
 */
int ts_txt_fetch(struct ts_context *ts, const struct ts_data_object *endpoint);

/**
 * FETCH request (binary mode).
 *
 * Read data object values (function called with an array as argument)
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the endpoint data object or NULL for root and special endpoints
 *                 0x16 ("_ids") and 0x17 ("_paths") as they don't actually exist in the database.
 * @param ret_type Return type flags (IDs, names and/or values).
 * @param pos_payload Position of payload in req buffer.
 */
int ts_bin_fetch(struct ts_context *ts, const struct ts_data_object *endpoint, uint32_t ret_type,
                 unsigned int pos_payload);

/**
 * PATCH request (text mode).
 *
 * Write data object values in text mode (function called with a map as argument)
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to the endpoint data object.
 */
int ts_txt_patch(struct ts_context *ts, const struct ts_data_object *endpoint);

/**
 * PATCH request (binary mode).
 *
 * Write data object values in binary mode (function called with a map as payload)
 *
 * If subset is specified, objects not found are silently ignored. Otherwise, a NOT_FOUND
 * error is raised.
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to endpoint object or NULL to consider any object
 * @param pos_payload Position of payload in req buffer
 * @param auth_flags Bitset to specify authentication status for different roles
 * @param subsets Bitset to specifiy data item subsets to be considered, 0 to ignore
 */
int ts_bin_patch(struct ts_context *ts, const struct ts_data_object *endpoint,
                 unsigned int pos_payload, uint8_t auth_flags, uint16_t subsets);

/**
 * POST request to append data.
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to subset object where a new value shall be created.
 */
int ts_txt_create(struct ts_context *ts, const struct ts_data_object *endpoint);

/**
 * DELETE request to delete data from object.
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to subset object from which a value shall be deleted.
 *
 * @returns Length of response message in buffer or 0 in case of error.
 */
int ts_txt_delete(struct ts_context *ts, const struct ts_data_object *endpoint);

/**
 * Execute command in text mode.
 *
 * Function called with a single data object name as argument.
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to executable object which shall be called.
 *
 * @returns Length of response message in buffer or 0 in case of error.
 */
int ts_txt_exec(struct ts_context *ts, const struct ts_data_object *endpoint);

/**
 * Execute command in binary mode (function called with a single data object name/id as argument).
 *
 * @param ts Pointer to ThingSet context.
 * @param endpoint Pointer to executable object which shall be called.
 * @param pos_payload Position of payload in req buffer
 *
 * @returns Length of response message in buffer or 0 in case of error.
 */
int ts_bin_exec(struct ts_context *ts, const struct ts_data_object *endpoint,
                unsigned int pos_payload);

/**
 * Fill the resp buffer with a JSON response status message.
 *
 * @param ts Pointer to ThingSet context.
 * @param code Numeric status code.
 *
 * @returns Length of status message in buffer or 0 in case of error.
 */
int ts_txt_response(struct ts_context *ts, int code);

/**
 * Fill the resp buffer with a CBOR response status message.
 *
 * @param ts Pointer to ThingSet context.
 * @param code Numeric status code.
 *
 * @returns Length of status message in buffer or 0 in case of error.
 */
int ts_bin_response(struct ts_context *ts, uint8_t code);

/**
 * Serialize a object value into a JSON string.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the JSON value should be stored.
 * @param size Size of the buffer, i.e. maximum allowed length of the value.
 * @param object Pointer to object which should be serialized.
 *
 * @returns Length of data written to buffer or 0 in case of error.
 */
int ts_json_serialize_value(struct ts_context *ts, char *buf, size_t size,
                            const struct ts_data_object *object);

/**
 * Serialize object name and value as JSON object.
 *
 * Same as ts_json_serialize_value, just that the object name is also serialized.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the JSON value should be stored.
 * @param size Size of the buffer, i.e. maximum allowed length of the value.
 * @param object Pointer to object which should be serialized.
 *
 * @returns Length of data written to buffer or 0 in case of error.
 */
int ts_json_serialize_name_value(struct ts_context *ts, char *buf, size_t size,
                                 const struct ts_data_object *object);

/**
 * Deserialize a object value from a JSON string.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the position of the value in a buffer.
 * @param len Length of value in the buffer.
 * @param type Type of the JSMN token as identified by the parser.
 * @param object Pointer to object where the deserialized value should be stored.
 *
 * @returns Number of tokens processed (always 1) or 0 in case of error.
 */
int ts_json_deserialize_value(struct ts_context *ts, char *buf, size_t len, jsmntype_t type,
                              const struct ts_data_object *object);

/**
 * Write the path of an object into a buffer.
 *
 * This function supports a maximum depth of 2 only (e.g. Meas/Bat_V).
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the JSON value should be stored.
 * @param size Size of the buffer, i.e. maximum allowed length of the value.
 * @param object Pointer to object which should be serialized.
 *
 * @returns Length of the string written to the buffer or value <= 0 in case of error.
 */
int ts_get_path(struct ts_context *ts, char *buf, size_t size, const struct ts_data_object *object);

/**
 * Get the endpoint object of a provided path.
 *
 * Similar to ts_get_object_by_path, but considers the number at the end if requesting a record
 *
 * @param ts Pointer to ThingSet context.
 * @param path Path with multiple object names separated by forward slash.
 * @param len Length of the entire path
 * @param index Pointer to a variable used to store the index of a record.
 *
 * @returns Pointer to data object or NULL if object is not found
 */
struct ts_data_object *ts_get_endpoint_by_path(struct ts_context *ts, const char *path, size_t len,
                                               uint16_t *index);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THINGSET_PRIV_H_ */
