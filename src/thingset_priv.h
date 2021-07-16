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

/**
 * Internal return type flags for payload data
 */
#define TS_RET_IDS              (1U << 0)
#define TS_RET_NAMES            (1U << 1)
#define TS_RET_VALUES           (1U << 2)

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
 * List child data nodes (function called without content / parameters)
 *
 * @param ts Pointer to ThingSet context.
 */
int ts_txt_get(struct ts_context *ts, const struct ts_data_node *parent_node, uint32_t ret_type);

/**
 * GET request (binary mode).
 *
 * List child data nodes (function called without content)
 *
 * @param ts Pointer to ThingSet context.
 */
int ts_bin_get(struct ts_context *ts, const struct ts_data_node *parent, uint32_t ret_type);

/**
 * FETCH request (text mode).
 *
 * Read data node values (function called with an array as argument)
 *
 * @param ts Pointer to ThingSet context.
 */
int ts_txt_fetch(struct ts_context *ts, ts_node_id_t parent_id);

/**
 * FETCH request (binary mode).
 *
 * Read data node values (function called with an array as argument)
 *
 * @param ts Pointer to ThingSet context.
 */
int ts_bin_fetch(struct ts_context *ts, const struct ts_data_node *parent, uint32_t ret_type,
                 unsigned int pos_payload);

/**
 * PATCH request (text mode).
 *
 * Write data node values in text mode (function called with a map as argument)
 *
 * @param ts Pointer to ThingSet context.
 */
int ts_txt_patch(struct ts_context *ts, ts_node_id_t parent_id);

/**
 * PATCH request (binary mode).
 *
 * Write data node values in binary mode (function called with a map as payload)
 *
 * If sub_ch is specified, nodes not found are silently ignored. Otherwise, a NOT_FOUND
 * error is raised.
 *
 * @param ts Pointer to ThingSet context.
 * @param parent Pointer to path / parent node or NULL to consider any node
 * @param pos_payload Position of payload in req buffer
 * @param auth_flags Bitset to specify authentication status for different roles
 * @param sub_ch Bitset to specifiy subscribe channel to be considered, 0 to ignore
 */
int ts_bin_patch(struct ts_context *ts, const struct ts_data_node *parent, unsigned int pos_payload, uint16_t auth_flags,
uint16_t sub_ch);

/**
 * POST request to append data.
 *
 * @param ts Pointer to ThingSet context.
 */
int ts_txt_create(struct ts_context *ts, const struct ts_data_node *node);

/**
 * DELETE request to delete data from node.
 *
 * @param ts Pointer to ThingSet context.
 */
int ts_txt_delete(struct ts_context *ts, const struct ts_data_node *node);

/**
 * Execute command in text mode.
 *
 * Function called with a single data node name as argument.
 *
 * @param ts Pointer to ThingSet context.
 */
int ts_txt_exec(struct ts_context *ts, const struct ts_data_node *node);

/**
 * Execute command in binary mode (function called with a single data node name/id as argument).
 *
 * @param ts Pointer to ThingSet context.
 * @param parent Pointer to executable node
 * @param pos_payload Position of payload in req buffer
 */
int ts_bin_exec(struct ts_context *ts, const struct ts_data_node *node, unsigned int pos_payload);

/**
 * Fill the resp buffer with a JSON response status message.
 *
 * @param ts Pointer to ThingSet context.
 * @param code Status code
 * @returns length of status message in buffer or 0 in case of error
 */
int ts_txt_response(struct ts_context *ts, int code);

/**
 * Fill the resp buffer with a CBOR response status message.
 *
 * @param ts Pointer to ThingSet context.
 * @param code Status code
 * @returns length of status message in buffer or 0 in case of error
 */
int ts_bin_response(struct ts_context *ts, uint8_t code);

/**
 * Serialize a node value into a JSON string.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the buffer where the JSON value should be stored
 * @param size Size of the buffer, i.e. maximum allowed length of the value
 * @param node Pointer to node which should be serialized
 *
 * @returns Length of data written to buffer or 0 in case of error
 */
int ts_json_serialize_value(struct ts_context *ts, char *buf, size_t size, const struct ts_data_node *node);

/**
 * Serialize node name and value as JSON object.
 *
 * same as ts_priv_json_serialize_value, just that the node name is also serialized
 */
int ts_json_serialize_name_value(struct ts_context *ts, char *buf, size_t size, const struct ts_data_node *node);

/**
 * Deserialize a node value from a JSON string.
 *
 * @param ts Pointer to ThingSet context.
 * @param buf Pointer to the position of the value in a buffer
 * @param len Length of value in the buffer
 * @param type Type of the JSMN token as identified by the parser
 * @param node Pointer to node where the deserialized value should be stored
 *
 * @returns Number of tokens processed (always 1) or 0 in case of error
 */
int ts_json_deserialize_value(struct ts_context *ts, char *buf, size_t len, jsmntype_t type, const struct ts_data_node *node);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* THINGSET_PRIV_H_ */
