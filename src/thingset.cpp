/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#include "ts_config.h"
#include "thingset.h"
#include "jsmn.h"
#include "cbor.h"

#include <string.h>
#include <stdio.h>

#define DEBUG 0

static void _check_id_duplicates(const DataNode *data, size_t num)
{
    for (unsigned int i = 0; i < num; i++) {
        for (unsigned int j = i + 1; j < num; j++) {
            if (data[i].id == data[j].id) {
                printf("ThingSet error: Duplicate data node ID 0x%X.\n", data[i].id);
            }
        }
    }
}

ThingSet::ThingSet(const DataNode *data, size_t num)
{
    _check_id_duplicates(data, num);
    data_nodes = data;
    num_nodes = num;
}

ThingSet::~ThingSet()
{

}

int ThingSet::process(uint8_t *request, size_t request_len, uint8_t *response, size_t response_size)
{
    // check if proper request was set before asking for a response
    if (request == NULL || request_len < 1)
        return 0;

    // assign private variables
    req = request;
    req_len = request_len;
    resp = response;
    resp_size = response_size;

    if (req[0] <= TS_EXEC) {          // CBOR list/read/write request
        if (req_len == 2 && (req[1] == CBOR_NULL || req[1] == CBOR_ARRAY || req[1] == CBOR_MAP)) {
            //printf("get_cbor\n");
            return get_cbor(req[0], req[1] == CBOR_MAP, req[1] == CBOR_NULL);
        }
        else if ((req[1] & CBOR_TYPE_MASK) == CBOR_MAP) {
            //printf("patch_cbor\n");
            int len = patch_cbor(req[0], false);
            if (response[0] == TS_STATUS_CHANGED &&
                req[0] == TS_CONF && conf_callback != NULL) {
                conf_callback();
            }
            return len;
        }
        else {  // array or single data node
            if (req[0] == TS_EXEC) {
                return exec_cbor();
            }
            else {
                //printf("fetch_cbor\n");
                return fetch_cbor(req[0]);
            }
        }
    }
    else if (req[0] == '?' || req[0] == '=' || req[0] == '+' || req[0] == '-' || req[0] == '!') {
        return process_json();
    }
    else {
        // not a thingset command --> ignore and set response to empty string
        response[0] = 0;
        return 0;
    }
}

void ThingSet::set_conf_callback(void (*callback)(void))
{
    conf_callback = callback;
}

const DataNode* ThingSet::get_data_node(const char *str, size_t len, int32_t parent)
{
    for (unsigned int i = 0; i < num_nodes; i++) {
        if (parent != -1 && data_nodes[i].parent != parent) {
            continue;
        }
        else if (strncmp(data_nodes[i].name, str, len) == 0
            && strlen(data_nodes[i].name) == len)  // otherwise e.g. foo and fooBar would be recognized as equal
        {
            return &(data_nodes[i]);
        }
    }
    return NULL;
}

const DataNode* ThingSet::get_data_node(uint16_t id)
{
    for (unsigned int i = 0; i < num_nodes; i++) {
        if (data_nodes[i].id == id) {
            return &(data_nodes[i]);
        }
    }
    return NULL;
}

const DataNode *ThingSet::get_endpoint_node(const char *path, size_t len)
{
    const DataNode *node;
    const char *start = path;
    const char *end = strchr(path, '/');
    uint16_t parent = 0;

    // maximum depth of 10 assumed
    for (int i = 0; i < 10; i++) {
        if (end != NULL) {
            if (end - path != len - 1) {
                node = get_data_node(start, end - start, parent);
                if (!node) {
                    return NULL;
                }
                parent = node->id;
                start = end + 1;
                end = strchr(start, '/');
            }
            else {
                // resource ends with trailing slash
                return get_data_node(start, end - start, parent);
            }
        }
        else {
            return get_data_node(start, path + len - start, parent);
        }
    }
    return NULL;
}

void ThingSet::set_user_password(const char *password)
{
    user_pass = password;
}

void ThingSet::set_maker_password(const char *password)
{
    maker_pass = password;
}
