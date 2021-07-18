/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#ifndef THINGSET_CPP_H_
#define THINGSET_CPP_H_

#include "jsmn.h"

/**
 * Main ThingSet class
 *
 * Stores and handles all data exposed to different communication interfaces
 */
class ThingSet
{
public:
    /**
     * Initialize a ThingSet object
     *
     * @param data Pointer to array of DataNode type containing the entire node database
     * @param num Number of elements in that array
     */
    ThingSet(DataNode *data, size_t num);

    /**
     * Process ThingSet request
     *
     * This function also detects if JSON or CBOR format is used
     *
     * @param request Pointer to the ThingSet request buffer
     * @param req_len Length of the data in the request buffer
     * @param response Pointer to the buffer where the ThingSet response should be stored
     * @param resp_size Size of the response buffer, i.e. maximum allowed length of the response

     * @returns Actual length of the response written to the buffer or 0 in case of error
     */
    int process(uint8_t *request, size_t req_len, uint8_t *response, size_t resp_size);

    /**
     * Print all data nodes as a structured JSON text to stdout
     *
     * WARNING: This is a recursive function and might cause stack overflows if run in constrained
     *          devices with large data node tree. Use with care and for testing only!
     *
     * @param node_id Root node ID where to start with printing
     * @param level Indentation level (=depth inside the data node tree)
     */
    void dump_json(node_id_t node_id = 0, int level = 0);

    /**
     * Sets current authentication level
     *
     * The authentication flags must match with access flags specified in DataNode to allow
     * read/write access to a data node.
     *
     * @param flags Flags to define authentication level (1 = access allowed)
     */
    void set_authentication(uint16_t flags)
    {
        _auth_flags = flags;
    }

    /**
     * Generate publication message in JSON format
     *
     * @param buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param pub_ch Flag to select publication channel (must match pubsub of data node)
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    int txt_pub(char *buf, size_t size, const uint16_t pub_ch);

    /**
     * Generate publication message in CBOR format
     *
     * @param buf Pointer to the buffer where the publication message should be stored
     * @param size Size of the message buffer, i.e. maximum allowed length of the message
     * @param pub_ch Flag to select publication channel (must match pubsub of data node)
     *
     * @returns Actual length of the message written to the buffer or 0 in case of error
     */
    int bin_pub(uint8_t *buf, size_t size, const uint16_t pub_ch);

    /**
     * Encode a publication message in CAN message format for supplied data node
     *
     * The data may only be 8 bytes long. If the actual length of a node exceeds the available
     * length, the node is silently ignored and the function continues with the next one.
     *
     * @param start_pos Position in data_nodes array to start searching
     *                  This value is updated with the next node found to allow iterating over all
     *                  nodes for this channel. It should be set to 0 to start from the beginning.
     * @param pub_ch Flag to select publication channel (must match pubsub of data node)
     * @param can_dev_id Device ID on the CAN bus
     * @param msg_id reference to can message id storage
     * @param msg_data reference to the buffer where the publication message should be stored
     *
     * @returns Actual length of the message_data or -1 if not encodable / in case of error
     */
    int bin_pub_can(int &start_pos, uint16_t pub_ch, uint8_t can_dev_id, uint32_t &msg_id,
        uint8_t (&msg_data)[8]);

    /**
     * Update data nodes based on values provided in payload data (e.g. from other pub msg)
     *
     * @param cbor_data Buffer containing key/value map that should be written to the data nodes
     * @param len Length of the data in the buffer
     * @param auth_flags Authentication flags to be used in this function (to override _auth_flags)
     * @param sub_ch Subscribe channel (as bitfield)
     *
     * @returns ThingSet status code
     */
    int bin_sub(uint8_t *cbor_data, size_t len, uint16_t auth_flags, uint16_t sub_ch);

    /**
     * Get data node by ID
     *
     * @param id Node ID
     *
     * @returns Pointer to data node or NULL if node is not found
     */
    DataNode *const get_node(node_id_t id);

    /**
     * Get data node by name
     *
     * As the names are not necessarily unique in the entire data tree, the parent is needed
     *
     * @param name Node name
     * @param len Length of the node name
     * @param parent Node ID of the parent or -1 for global search
     *
     * @returns Pointer to data node or NULL if node is not found
     */
    DataNode *const get_node(const char *name, size_t len, int32_t parent = -1);

    /**
     * Get the endpoint node of a provided path
     *
     * @param path Path with multiple node names separated by forward slash
     * @param len Length of the entire path
     *
     * @returns Pointer to data node or NULL if node is not found
     */
    DataNode *const get_endpoint(const char *path, size_t len);

private:
    /**
     * Prepares JSMN parser, performs initial check of payload data and calls get/fetch/patch
     * functions
     */
    int txt_process();

    /**
     * Performs initial check of payload data and calls get/fetch/patch functions
     */
    int bin_process();

    /**
     * GET request (text mode)
     *
     * List child data nodes (function called without content / parameters)
     */
    int txt_get(const DataNode *parent, bool include_values = false);

    /**
     * GET request (binary mode)
     *
     * List child data nodes (function called without content)
     */
    int bin_get(const DataNode *parent, bool values = false, bool ids_only = true);

    /**
     * FETCH request (text mode)
     *
     * Read data node values (function called with an array as argument)
     */
    int txt_fetch(node_id_t parent_id);

    /**
     * FETCH request (binary mode)
     *
     * Read data node values (function called with an array as argument)
     */
    int bin_fetch(const DataNode *parent, unsigned int pos_payload);

    /**
     * PATCH request (text mode)
     *
     * Write data node values in text mode (function called with a map as argument)
     */
    int txt_patch(node_id_t parent_id);

    /**
     * PATCH request (binary mode)
     *
     * Write data node values in binary mode (function called with a map as payload)
     *
     * If sub_ch is specified, nodes not found are silently ignored. Otherwise, a NOT_FOUND
     * error is raised.
     *
     * @param parent Pointer to path / parent node or NULL to consider any node
     * @param pos_payload Position of payload in req buffer
     * @param auth_flags Bitset to specify authentication status for different roles
     * @param sub_ch Bitset to specifiy subscribe channel to be considered, 0 to ignore
     */
    int bin_patch(const DataNode *parent, unsigned int pos_payload, uint16_t auth_flags,
        uint16_t sub_ch);

    /**
     * POST request to append data
     */
    int txt_create(const DataNode *node);

    /**
     * DELETE request to delete data from node
     */
    int txt_delete(const DataNode *node);

    /**
     * Execute command in text mode (function called with a single data node name as argument)
     */
    int txt_exec(const DataNode *node);

    /**
     * Execute command in binary mode (function called with a single data node name/id as argument)
     *
     * @param parent Pointer to executable node
     * @param pos_payload Position of payload in req buffer
     */
    int bin_exec(const DataNode *node, unsigned int pos_payload);

    /**
     * Fill the resp buffer with a JSON response status message
     *
     * @param code Status code
     * @returns length of status message in buffer or 0 in case of error
     */
    int txt_response(int code);

    /**
     * Fill the resp buffer with a CBOR response status message
     *
     * @param code Status code
     * @returns length of status message in buffer or 0 in case of error
     */
    int bin_response(uint8_t code);

    /**
     * Serialize a node value into a JSON string
     *
     * @param buf Pointer to the buffer where the JSON value should be stored
     * @param size Size of the buffer, i.e. maximum allowed length of the value
     * @param node Pointer to node which should be serialized
     *
     * @returns Length of data written to buffer or 0 in case of error
     */
    int json_serialize_value(char *buf, size_t size, const DataNode *node);

    /**
     * Serialize node name and value as JSON object
     *
     * same as json_serialize_value, just that the node name is also serialized
     */
    int json_serialize_name_value(char *buf, size_t size, const DataNode *node);

    /**
     * Deserialize a node value from a JSON string
     *
     * @param buf Pointer to the position of the value in a buffer
     * @param len Length of value in the buffer
     * @param type Type of the JSMN token as identified by the parser
     * @param node Pointer to node where the deserialized value should be stored
     *
     * @returns Number of tokens processed (always 1) or 0 in case of error
     */
    int json_deserialize_value(char *buf, size_t len, jsmntype_t type, const DataNode *node);

    /**
     * Array of nodes database provided during initialization
     */
    DataNode *data_nodes;

    /**
     * Number of nodes in the data_nodes array
     */
    size_t num_nodes;

    /**
     * Pointer to request buffer (provided in process function)
     */
    uint8_t *req;

    /**
     * Length of the request
     */
    size_t req_len;

    /**
     * Pointer to response buffer (provided in process function)
     */
    uint8_t *resp;

    /**
     * Size of response buffer (i.e. maximum length)
     */
    size_t resp_size;

    /**
     * Pointer to the start of JSON payload in the request
     */
    char *json_str;

    /**
     * JSON tokes in json_str parsed by JSMN
     */
    jsmntok_t tokens[TS_NUM_JSON_TOKENS];

    /**
     * Number of JSON tokens parsed by JSMN
     */
    int tok_count;

    /**
     * Stores current authentication status (authentication as "normal" user as default)
     */
    uint16_t _auth_flags = TS_USR_MASK;
};

#endif /* THINGSET_CPP_H_ */
