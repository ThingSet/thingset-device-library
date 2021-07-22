/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* The main implementation file of the thingset library. */
#define THINGSET_MAIN 1

#include "thingset_priv.h"

#include <stdio.h>


static void _check_id_duplicates(const TsDataNode *data, size_t num)
{
    for (unsigned int i = 0; i < num; i++) {
        for (unsigned int j = i + 1; j < num; j++) {
            if (data[i].id == data[j].id) {
                LOG_ERR("ThingSet error: Duplicate data node ID 0x%X.\n", data[i].id);
            }
        }
    }
}

/*
 * Counts the number of elements in an an array of node IDs by looking for the first non-zero
 * elements starting from the back.
 *
 * Currently only supporting uint16_t (node_id_t) arrays as we need the size of each element
 * to iterate through the array.
 */
static void _count_array_elements(const TsDataNode *data, size_t num)
{
    for (unsigned int i = 0; i < num; i++) {
        if (data[i].type == TS_T_ARRAY) {
            TsArrayInfo *arr = (TsArrayInfo *)data[i].data;
            if (arr->num_elements == TS_AUTODETECT_ARRLEN) {
                arr->num_elements = 0;  // set to safe default
                if (arr->type == TS_T_NODE_ID) {
                    for (int elem = arr->max_elements - 1; elem >= 0; elem--) {
                        if (((ts_node_id_t *)arr->ptr)[elem] != 0) {
                            arr->num_elements = elem + 1;
                            LOG_DBG("%s num elements: %d\n", data[i].name, arr->num_elements);
                            break;
                        }
                    }
                }
                else {
                    LOG_ERR("Autodetecting array length of node 0x%X not possible.\n", data[i].id);
                }
            }
        }
    }
}

int ts_init(ts_object_t *ts, TsDataNode *data, size_t num)
{
    _check_id_duplicates(data, num);

    _count_array_elements(data, num);

    ts->data_nodes = data;
    ts->num_nodes = num;
    ts->_auth_flags = TS_USR_MASK;

    return 0;
}

int ts_process(ts_object_t *ts, uint8_t *request, size_t request_len, uint8_t *response, size_t response_size)
{
    // check if proper request was set before asking for a response
    if (request == NULL || request_len < 1)
        return 0;

    // assign private variables
    ts->req = request;
    ts->req_len = request_len;
    ts->resp = response;
    ts->resp_size = response_size;

    if (ts->req[0] < 0x20) {
        // binary mode request
        return ts_priv_bin_process(ts);
    } else if (ts->req[0] == '?' || ts->req[0] == '=' || ts->req[0] == '+'
               || ts->req[0] == '-' || ts->req[0] == '!') {
        // text mode request
        return ts_priv_txt_process(ts);
    } else {
        // not a thingset command --> ignore and set response to empty string
        response[0] = 0;
        return 0;
    }
}

TsDataNode *const ts_get_node_by_name(ts_object_t *ts, const char *name, size_t len, int32_t parent)
{
    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (parent != -1 && ts->data_nodes[i].parent != parent) {
            continue;
        }
        else if (strncmp(ts->data_nodes[i].name, name, len) == 0
            && strlen(ts->data_nodes[i].name) == len)  // otherwise e.g. foo and fooBar would be recognized as equal
        {
            return &(ts->data_nodes[i]);
        }
    }
    return NULL;
}

TsDataNode *const ts_get_node_by_id(ts_object_t *ts, ts_node_id_t id)
{
    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].id == id) {
            return &(ts->data_nodes[i]);
        }
    }
    return NULL;
}

TsDataNode *const ts_get_node_by_path(ts_object_t *ts, const char *path, size_t len)
{
    TsDataNode *node;
    const char *start = path;
    const char *end;
    uint16_t parent = 0;

    // maximum depth of 10 assumed
    for (int i = 0; i < 10; i++) {
        end = strchr(start, '/');
        if (end == NULL || end >= path + len) {
            // we are at the end of the path
            return ts_get_node_by_name(ts, start, path + len - start, parent);
        }
        else if (end == path + len - 1) {
            // path ends with slash
            return ts_get_node_by_name(ts, start, end - start, parent);
        }
        else {
            // go further down the path
            node = ts_get_node_by_name(ts, start, end - start, parent);
            if (node) {
                parent = node->id;
                start = end + 1;
            }
            else {
                return NULL;
            }
        }
    }
    return NULL;
}
