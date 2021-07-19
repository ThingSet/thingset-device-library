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

#ifdef __ZEPHYR__

/* Logging */
#define LOG_MODULE_NAME thingset
#define LOG_LEVEL CONFIG_THINGSET_LOG_LEVEL
#include <logging/log.h>

#ifdef THINGSET_MAIN
LOG_MODULE_REGISTER(LOG_MODULE_NAME);
#else
LOG_MODULE_DECLARE(LOG_MODULE_NAME);
#endif

#define LOG_ALLOC_STR(str)	((str == NULL) ? log_strdup("null") : \
                                                log_strdup(str))

#include <zephyr.h>

#else /* ! __ZEPHYR */

#define DEBUG 0

#define LOG_DBG(...) printf(__VA_ARGS__)
#define LOG_ERR(...) printf(__VA_ARGS__)
#define LOG_ALLOC_STR(str) str

#include <stdio.h>

#endif /* __ZEPHYR */

/**
 * Prepares JSMN parser, performs initial check of payload data and calls get/fetch/patch
 * functions
 */
int ts_priv_txt_process(ts_object_t *ts);

/**
 * Performs initial check of payload data and calls get/fetch/patch functions
 */
int ts_priv_bin_process(ts_object_t *ts);

/**
 * GET request (text mode)
 *
 * List child data nodes (function called without content / parameters)
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 */
int ts_priv_txt_get(ts_object_t *ts, const TsDataNode *parent_node, bool include_values);

/**
 * GET request (binary mode).
 *
 * List child data nodes (function called without content)
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 */
int ts_priv_bin_get(ts_object_t *ts, const TsDataNode *parent, bool values, bool ids_only);

/**
 * FETCH request (text mode).
 *
 * Read data node values (function called with an array as argument)
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 */
int ts_priv_txt_fetch(ts_object_t *ts, ts_node_id_t parent_id);

/**
 * FETCH request (binary mode).
 *
 * Read data node values (function called with an array as argument)
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 */
int ts_priv_bin_fetch(ts_object_t *ts, const TsDataNode *parent, unsigned int pos_payload);

/**
 * PATCH request (text mode)
 *
 * Write data node values in text mode (function called with a map as argument)
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 */
int ts_priv_txt_patch(ts_object_t *ts, ts_node_id_t parent_id);

/**
 * PATCH request (binary mode)
 *
 * Write data node values in binary mode (function called with a map as payload)
 *
 * If sub_ch is specified, nodes not found are silently ignored. Otherwise, a NOT_FOUND
 * error is raised.
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 * @param parent Pointer to path / parent node or NULL to consider any node
 * @param pos_payload Position of payload in req buffer
 * @param auth_flags Bitset to specify authentication status for different roles
 * @param sub_ch Bitset to specifiy subscribe channel to be considered, 0 to ignore
 */
int ts_priv_bin_patch(ts_object_t *ts, const TsDataNode *parent, unsigned int pos_payload, uint16_t auth_flags,
uint16_t sub_ch);

/**
 * POST request to append data.
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 */
int ts_priv_txt_create(ts_object_t *ts, const TsDataNode *node);

/**
 * DELETE request to delete data from node.
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 */
int ts_priv_txt_delete(ts_object_t *ts, const TsDataNode *node);

/**
 * Execute command in text mode.
 *
 * Function called with a single data node name as argument.
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 */
int ts_priv_txt_exec(ts_object_t *ts, const TsDataNode *node);

/**
 * Execute command in binary mode (function called with a single data node name/id as argument).
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 * @param parent Pointer to executable node
 * @param pos_payload Position of payload in req buffer
 */
int ts_priv_bin_exec(ts_object_t *ts, const TsDataNode *node, unsigned int pos_payload);

/**
 * Fill the resp buffer with a JSON response status message
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 * @param code Status code
 * @returns length of status message in buffer or 0 in case of error
 */
int ts_priv_txt_response(ts_object_t *ts, int code);

/**
 * Fill the resp buffer with a CBOR response status message
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 * @param code Status code
 * @returns length of status message in buffer or 0 in case of error
 */
int ts_priv_bin_response(ts_object_t *ts, uint8_t code);

/**
 * Serialize a node value into a JSON string
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 * @param buf Pointer to the buffer where the JSON value should be stored
 * @param size Size of the buffer, i.e. maximum allowed length of the value
 * @param node Pointer to node which should be serialized
 *
 * @returns Length of data written to buffer or 0 in case of error
 */
int ts_priv_json_serialize_value(ts_object_t *ts, char *buf, size_t size, const TsDataNode *node);

/**
 * Serialize node name and value as JSON object
 *
 * same as ts_priv_json_serialize_value, just that the node name is also serialized
 */
int ts_priv_json_serialize_name_value(ts_object_t *ts, char *buf, size_t size, const TsDataNode *node);

/**
 * Deserialize a node value from a JSON string
 *
 * @param ts Pointer to ts_object_t type ThingSet object.
 * @param buf Pointer to the position of the value in a buffer
 * @param len Length of value in the buffer
 * @param type Type of the JSMN token as identified by the parser
 * @param node Pointer to node where the deserialized value should be stored
 *
 * @returns Number of tokens processed (always 1) or 0 in case of error
 */
int ts_priv_json_deserialize_value(ts_object_t *ts, char *buf, size_t len, jsmntype_t type, const TsDataNode *node);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THINGSET_PRIV_H_ */
