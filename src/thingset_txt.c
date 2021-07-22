/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "thingset_priv.h"

#include "jsmn.h"

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

int ts_priv_txt_response(ts_object_t *ts, int code)
{
    size_t pos = 0;
#ifdef TS_VERBOSE_STATUS_MESSAGES
    switch(code) {
        // success
        case TS_STATUS_CREATED:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Created.", code);
            break;
        case TS_STATUS_DELETED:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Deleted.", code);
            break;
        case TS_STATUS_VALID:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Valid.", code);
            break;
        case TS_STATUS_CHANGED:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Changed.", code);
            break;
        case TS_STATUS_CONTENT:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Content.", code);
            break;
        // client errors
        case TS_STATUS_BAD_REQUEST:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Bad Request.", code);
            break;
        case TS_STATUS_UNAUTHORIZED:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Unauthorized.", code);
            break;
        case TS_STATUS_FORBIDDEN:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Forbidden.", code);
            break;
        case TS_STATUS_NOT_FOUND:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Not Found.", code);
            break;
        case TS_STATUS_METHOD_NOT_ALLOWED:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Method Not Allowed.", code);
            break;
        case TS_STATUS_REQUEST_INCOMPLETE:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Request Entity Incomplete.", code);
            break;
        case TS_STATUS_CONFLICT:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Conflict.", code);
            break;
        case TS_STATUS_REQUEST_TOO_LARGE:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Request Entity Too Large.", code);
            break;
        case TS_STATUS_UNSUPPORTED_FORMAT:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Unsupported Content-Format.", code);
            break;
        // server errors
        case TS_STATUS_INTERNAL_SERVER_ERR:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Internal Server Error.", code);
            break;
        case TS_STATUS_NOT_IMPLEMENTED:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Not Implemented.", code);
            break;
        // ThingSet specific errors
        case TS_STATUS_RESPONSE_TOO_LARGE:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Response too large.", code);
            break;
        default:
            pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X Error.", code);
            break;
    };
#else
    pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X.", code);
#endif
    if (pos < ts->resp_size)
        return pos;
    else
        return 0;
}

int ts_priv_json_serialize_value(ts_object_t *ts, char *buf, size_t size, const TsDataNode *node)
{
    size_t pos = 0;
    const TsDataNode *sub_node;
    TsArrayInfo *array_info;
    float value;

    switch (node->type) {
#ifdef TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        pos = snprintf(&buf[pos], size - pos, "%" PRIu64 ",", *((uint64_t *)node->data));
        break;
    case TS_T_INT64:
        pos = snprintf(&buf[pos], size - pos, "%" PRIi64 ",", *((int64_t *)node->data));
        break;
#endif
    case TS_T_UINT32:
        pos = snprintf(&buf[pos], size - pos, "%" PRIu32 ",", *((uint32_t *)node->data));
        break;
    case TS_T_INT32:
        pos = snprintf(&buf[pos], size - pos, "%" PRIi32 ",", *((int32_t *)node->data));
        break;
    case TS_T_UINT16:
        pos = snprintf(&buf[pos], size - pos, "%" PRIu16 ",", *((uint16_t *)node->data));
        break;
    case TS_T_INT16:
        pos = snprintf(&buf[pos], size - pos, "%" PRIi16 ",", *((int16_t *)node->data));
        break;
    case TS_T_FLOAT32:
        value = *((float *)node->data);
        if (isnan(value) || isinf(value)) {
            /* JSON spec does not support NaN and Inf, so we need to use null instead */
            return snprintf(buf, size, "null,");
        }
        else {
            pos = snprintf(&buf[pos], size - pos, "%.*f,", node->detail, value);
        }
        break;
    case TS_T_BOOL:
        pos = snprintf(&buf[pos], size - pos, "%s,",
                (*((bool *)node->data) == true ? "true" : "false"));
        break;
    case TS_T_EXEC:
        pos = snprintf(&buf[pos], size - pos, "null,");
        break;
    case TS_T_STRING:
        pos = snprintf(&buf[pos], size - pos, "\"%s\",", (char *)node->data);
        break;
    case TS_T_PUBSUB:
        pos = snprintf(&buf[pos], size - pos, "[]") - 1;
        for (unsigned int i = 0; i < ts->num_nodes; i++) {
            if (ts->data_nodes[i].pubsub & (uint16_t)node->detail) {
                pos += snprintf(&buf[pos], size - pos, "\"%s\",", ts->data_nodes[i].name);
            }
        }
        pos--; // remove trailing comma
        pos += snprintf(&buf[pos], size - pos, "],");
        break;
    case TS_T_ARRAY:
        array_info = (TsArrayInfo *)node->data;
        if (!array_info) {
            return 0;
        }
        pos += snprintf(&buf[pos], size - pos, "[");
        for (int i = 0; i < array_info->num_elements; i++) {
            switch (array_info->type) {
            case TS_T_UINT64:
                pos += snprintf(&buf[pos], size - pos, "%" PRIu64 ",",
                        ((uint64_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT64:
                pos += snprintf(&buf[pos], size - pos, "%" PRIi64 ",",
                        ((int64_t *)array_info->ptr)[i]);
                break;
            case TS_T_UINT32:
                pos += snprintf(&buf[pos], size - pos, "%" PRIu32 ",",
                        ((uint32_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT32:
                pos += snprintf(&buf[pos], size - pos, "%" PRIi32 ",",
                        ((int32_t *)array_info->ptr)[i]);
                break;
            case TS_T_UINT16:
                pos += snprintf(&buf[pos], size - pos, "%" PRIu16 ",",
                        ((uint16_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT16:
                pos += snprintf(&buf[pos], size - pos, "%" PRIi16 ",",
                        ((int16_t *)array_info->ptr)[i]);
                break;
            case TS_T_FLOAT32:
                pos += snprintf(&buf[pos], size - pos, "%.*f,", node->detail,
                        ((float *)array_info->ptr)[i]);
                break;
            case TS_T_NODE_ID:
                sub_node = ts_get_node_by_id(ts, ((uint16_t *)array_info->ptr)[i]);
                if (sub_node) {
                    pos += snprintf(&buf[pos], size - pos, "\"%s\",", sub_node->name);
                }
                break;
            default:
                break;
            }
        }
        if (array_info->num_elements > 0) {
            pos--; // remove trailing comma
        }
        pos += snprintf(&buf[pos], size - pos, "],");
        break;
    }

    if (pos < size) {
        return pos;
    }
    else {
        return 0;
    }
}

int ts_priv_json_serialize_name_value(ts_object_t *ts, char *buf, size_t size, const TsDataNode *node)
{
    size_t pos = snprintf(buf, size, "\"%s\":", node->name);

    int len_value = ts_priv_json_serialize_value(ts, &buf[pos], size - pos, node);
    pos += len_value;

    if (len_value > 0 && pos < size) {
        return pos;
    }
    else {
        return 0;
    }
}

void ts_dump_json(ts_object_t *ts, ts_node_id_t node_id, int level)
{
    uint8_t buf[100];
    bool first = true;
    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].parent == node_id) {
            if (!first) {
                printf(",\n");
            }
            else {
                printf("\n");
                first = false;
            }
            if (ts->data_nodes[i].type == TS_T_PATH) {
                LOG_DBG("%*s\"%s\" {", 4 * level, "", ts->data_nodes[i].name);
                ts_dump_json(ts, ts->data_nodes[i].id, level + 1);
                LOG_DBG("\n%*s}", 4 * level, "");
            }
            else {
                int pos = ts_priv_json_serialize_name_value(ts, (char *)buf, sizeof(buf), &ts->data_nodes[i]);
                if (pos > 0) {
                    buf[pos-1] = '\0';  // remove trailing comma
                    LOG_DBG("%*s%s", 4 * level, "", LOG_ALLOC_STR((char *)buf));
                }
            }
        }
    }
    if (node_id == 0) {
        LOG_DBG("\n");
    }
}

int ts_priv_txt_process(ts_object_t *ts)
{
    int path_len = ts->req_len - 1;
    char *path_end = strchr((char *)ts->req + 1, ' ');
    if (path_end) {
        path_len = (uint8_t *)path_end - ts->req - 1;
    }

    const TsDataNode *endpoint = ts_get_node_by_path(ts, (char *)ts->req + 1, path_len);
    if (!endpoint) {
        if (ts->req[0] == '?' && ts->req[1] == '/' && path_len == 1) {
            return ts_priv_txt_get(ts, NULL, false);
        }
        else {
            return ts_priv_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
    }

    jsmn_parser parser;
    jsmn_init(&parser);

    ts->json_str = (char *)ts->req + 1 + path_len;
    ts->tok_count = jsmn_parse(&parser, ts->json_str, ts->req_len - path_len - 1, ts->tokens, sizeof(ts->tokens));

    if (ts->tok_count == JSMN_ERROR_NOMEM) {
        return ts_priv_txt_response(ts, TS_STATUS_REQUEST_TOO_LARGE);
    }
    else if (ts->tok_count < 0) {
        // other parsing error
        return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
    }
    else if (ts->tok_count == 0) {
        if (ts->req[0] == '?') {
            // no payload data
            if ((char)ts->req[path_len] == '/') {
                if (endpoint->type == TS_T_PATH || endpoint->type == TS_T_EXEC) {
                    return ts_priv_txt_get(ts, endpoint, false);
                }
                else {
                    // device discovery is only allowed for internal nodes
                    return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
                }
            }
            else {
                return ts_priv_txt_get(ts, endpoint, true);
            }
        }
        else if (ts->req[0] == '!') {
            return ts_priv_txt_exec(ts, endpoint);
        }
    }
    else {
        if (ts->req[0] == '?') {
            return ts_priv_txt_fetch(ts, endpoint->id);
        }
        else if (ts->req[0] == '=') {
            int len = ts_priv_txt_patch(ts, endpoint->id);

            // check if endpoint has a callback assigned
            if (endpoint->data != NULL && strncmp((char *)ts->resp, ":84", 3) == 0) {
                // create function pointer and call function
                void (*fun)(void) = (void(*)(void))endpoint->data;
                fun();
            }
            return len;
        }
        else if (ts->req[0] == '!' && endpoint->type == TS_T_EXEC) {
            return ts_priv_txt_exec(ts, endpoint);
        }
        else if (ts->req[0] == '+') {
            return ts_priv_txt_create(ts, endpoint);
        }
        else if (ts->req[0] == '-') {
            return ts_priv_txt_delete(ts, endpoint);
        }
    }
    return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
}

int ts_priv_txt_fetch(ts_object_t *ts, ts_node_id_t parent_id)
{
    size_t pos = 0;
    int tok = 0;       // current token

    // initialize response with success message
    pos += ts_priv_txt_response(ts, TS_STATUS_CONTENT);

    if (ts->tokens[0].type == JSMN_ARRAY) {
        pos += snprintf((char *)&ts->resp[pos], ts->resp_size - pos, " [");
        tok++;
    } else {
        pos += snprintf((char *)&ts->resp[pos], ts->resp_size - pos, " ");
    }

    while (tok < ts->tok_count) {

        if (ts->tokens[tok].type != JSMN_STRING) {
            return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
        }

        const TsDataNode *node = ts_get_node_by_name(ts,
            ts->json_str + ts->tokens[tok].start,
            ts->tokens[tok].end - ts->tokens[tok].start, parent_id);

        if (node == NULL) {
            return ts_priv_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
        else if (node->type == TS_T_PATH) {
            // bad request, as we can't read internal path node's values
            return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
        }

        if ((node->access & TS_READ_MASK & ts->_auth_flags) == 0) {
            if (node->access & TS_READ_MASK) {
                return ts_priv_txt_response(ts, TS_STATUS_UNAUTHORIZED);
            }
            else {
                return ts_priv_txt_response(ts, TS_STATUS_FORBIDDEN);
            }
        }

        pos += ts_priv_json_serialize_value(ts, (char *)&ts->resp[pos], ts->resp_size - pos, node);

        if (pos >= ts->resp_size - 2) {
            return ts_priv_txt_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
        }
        tok++;
    }

    pos--;  // remove trailing comma
    if (ts->tokens[0].type == JSMN_ARRAY) {
        // buffer will be long enough as we dropped last 2 characters --> sprintf allowed
        pos += sprintf((char *)&ts->resp[pos], "]");
    } else {
        ts->resp[pos] = '\0';    // terminate string
    }

    return pos;
}

int ts_priv_json_deserialize_value(ts_object_t *ts, char *buf, size_t len, jsmntype_t type, const TsDataNode *node)
{
    if (type != JSMN_PRIMITIVE && type != JSMN_STRING) {
        return 0;
    }

    errno = 0;
    switch (node->type) {
        case TS_T_FLOAT32:
            *((float*)node->data) = strtod(buf, NULL);
            break;
        case TS_T_UINT64:
            *((uint64_t*)node->data) = strtoull(buf, NULL, 0);
            break;
        case TS_T_INT64:
            *((int64_t*)node->data) = strtoll(buf, NULL, 0);
            break;
        case TS_T_UINT32:
            *((uint32_t*)node->data) = strtoul(buf, NULL, 0);
            break;
        case TS_T_INT32:
            *((int32_t*)node->data) = strtol(buf, NULL, 0);
            break;
        case TS_T_UINT16:
            *((uint16_t*)node->data) = strtoul(buf, NULL, 0);
            break;
        case TS_T_INT16:
            *((uint16_t*)node->data) = strtol(buf, NULL, 0);
            break;
        case TS_T_BOOL:
            if (buf[0] == 't' || buf[0] == '1') {
                *((bool*)node->data) = true;
            }
            else if (buf[0] == 'f' || buf[0] == '0') {
                *((bool*)node->data) = false;
            }
            else {
                return 0;       // error
            }
            break;
        case TS_T_STRING:
            if (type != JSMN_STRING || (unsigned int)node->detail <= len) {
                return 0;
            }
            else if (node->id != 0) {     // dummy node has id = 0
                strncpy((char*)node->data, buf, len);
                ((char*)node->data)[len] = '\0';
            }
            break;
    }

    if (errno == ERANGE) {
        return 0;
    }

    return 1;   // value always contained in one token (arrays not yet supported)
}

int ts_priv_txt_patch(ts_object_t *ts, ts_node_id_t parent_id)
{
    int tok = 0;       // current token

    // buffer for data node value (largest negative 64bit integer has 20 digits)
    char value_buf[21];
    size_t value_len;   // length of value in buffer

    if (ts->tok_count < 2) {
        if (ts->tok_count == JSMN_ERROR_NOMEM) {
            return ts_priv_txt_response(ts, TS_STATUS_REQUEST_TOO_LARGE);
        } else {
            return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
        }
    }

    if (ts->tokens[0].type == JSMN_OBJECT) {    // object = map
        tok++;
    }

    // loop through all elements to check if request is valid
    while (tok + 1 < ts->tok_count) {

        if (ts->tokens[tok].type != JSMN_STRING ||
            (ts->tokens[tok+1].type != JSMN_PRIMITIVE && ts->tokens[tok+1].type != JSMN_STRING)) {
            return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
        }

        const TsDataNode* node = ts_get_node_by_name(ts,
            ts->json_str + ts->tokens[tok].start,
            ts->tokens[tok].end - ts->tokens[tok].start, parent_id);

        if (node == NULL) {
            return ts_priv_txt_response(ts, TS_STATUS_NOT_FOUND);
        }

        if ((node->access & TS_WRITE_MASK & ts->_auth_flags) == 0) {
            if (node->access & TS_WRITE_MASK) {
                return ts_priv_txt_response(ts, TS_STATUS_UNAUTHORIZED);
            }
            else {
                return ts_priv_txt_response(ts, TS_STATUS_FORBIDDEN);
            }
        }

        tok++;

        // extract the value and check buffer lengths
        value_len = ts->tokens[tok].end - ts->tokens[tok].start;
        if ((node->type != TS_T_STRING && value_len >= sizeof(value_buf)) ||
            (node->type == TS_T_STRING && value_len >= (size_t)node->detail))
        {
            return ts_priv_txt_response(ts, TS_STATUS_UNSUPPORTED_FORMAT);
        }
        else {
            strncpy(value_buf, &ts->json_str[ts->tokens[tok].start], value_len);
            value_buf[value_len] = '\0';
        }

        // create dummy node to test formats
        uint8_t dummy_data[8];          // enough to fit also 64-bit values
        TsDataNode dummy_node = {0, 0, "Dummy", (void *)dummy_data, node->type, node->detail};

        int res = ts_priv_json_deserialize_value(ts, value_buf, value_len, ts->tokens[tok].type, &dummy_node);
        if (res == 0) {
            return ts_priv_txt_response(ts, TS_STATUS_UNSUPPORTED_FORMAT);
        }
        tok += res;
    }

    if (ts->tokens[0].type == JSMN_OBJECT) {
        tok = 1;
    }
    else {
        tok = 0;
    }

    // actually write data
    while (tok + 1 < ts->tok_count) {

        const TsDataNode *node = ts_get_node_by_name(ts, ts->json_str + ts->tokens[tok].start,
            ts->tokens[tok].end - ts->tokens[tok].start, parent_id);

        tok++;

        // extract the value again (max. size was checked before)
        value_len = ts->tokens[tok].end - ts->tokens[tok].start;
        if (value_len < sizeof(value_buf)) {
            strncpy(value_buf, &ts->json_str[ts->tokens[tok].start], value_len);
            value_buf[value_len] = '\0';
        }

        tok += ts_priv_json_deserialize_value(ts, &ts->json_str[ts->tokens[tok].start], value_len, ts->tokens[tok].type,
            node);
    }

    return ts_priv_txt_response(ts, TS_STATUS_CHANGED);
}

int ts_priv_txt_get(ts_object_t *ts, const TsDataNode *parent_node, bool include_values)
{
    // initialize response with success message
    size_t len = ts_priv_txt_response(ts, TS_STATUS_CONTENT);

    ts_node_id_t parent_node_id = (parent_node == NULL) ? 0 : parent_node->id;

    if (parent_node != NULL && parent_node->type != TS_T_PATH &&
        parent_node->type != TS_T_EXEC)
    {
        // get value of data node
        ts->resp[len++] = ' ';
        len += ts_priv_json_serialize_value(ts, (char *)&ts->resp[len], ts->resp_size - len, parent_node);
        ts->resp[--len] = '\0';     // remove trailing comma again
        return len;
    }

    if (parent_node != NULL && parent_node->type == TS_T_EXEC && include_values) {
        // bad request, as we can't read exec node's values
        return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
    }

    len += sprintf((char *)&ts->resp[len], include_values ? " {" : " [");
    int nodes_found = 0;
    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if ((ts->data_nodes[i].access & TS_READ_MASK) &&
            (ts->data_nodes[i].parent == parent_node_id))
        {
            if (include_values) {
                if (ts->data_nodes[i].type == TS_T_PATH) {
                    // bad request, as we can't read nternal path node's values
                    return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
                }
                int ret = ts_priv_json_serialize_name_value(ts, (char *)&ts->resp[len], ts->resp_size - len,
                    &ts->data_nodes[i]);
                if (ret > 0) {
                    len += ret;
                }
                else {
                    return ts_priv_txt_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
                }
            }
            else {
                len += snprintf((char *)&ts->resp[len],
                    ts->resp_size - len,
                    "\"%s\",", ts->data_nodes[i].name);
            }
            nodes_found++;

            if (len >= ts->resp_size - 1) {
                return ts_priv_txt_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
            }
        }
    }

    // remove trailing comma and add closing bracket
    if (nodes_found == 0) {
        len++;
    }
    ts->resp[len-1] = include_values ? '}' : ']';
    ts->resp[len] = '\0';

    return len;
}

int ts_priv_txt_create(ts_object_t *ts, const TsDataNode *node)
{
    if (ts->tok_count > 1) {
        // only single JSON primitive supported at the moment
        return ts_priv_txt_response(ts, TS_STATUS_NOT_IMPLEMENTED);
    }

    if (node->type == TS_T_ARRAY) {
        TsArrayInfo *arr_info = (TsArrayInfo *)node->data;
        if (arr_info->num_elements < arr_info->max_elements) {

            if (arr_info->type == TS_T_NODE_ID && ts->tokens[0].type == JSMN_STRING) {

                const TsDataNode *new_node = ts_get_node_by_name(ts, ts->json_str + ts->tokens[0].start,
                    ts->tokens[0].end - ts->tokens[0].start, -1);

                if (new_node != NULL) {
                    ts_node_id_t *node_ids = (ts_node_id_t *)arr_info->ptr;
                    // check if node is already existing in array
                    for (int i = 0; i < arr_info->num_elements; i++) {
                        if (node_ids[i] == new_node->id) {
                            return ts_priv_txt_response(ts, TS_STATUS_CONFLICT);
                        }
                    }
                    // otherwise append it
                    node_ids[arr_info->num_elements] = new_node->id;
                    arr_info->num_elements++;
                    return ts_priv_txt_response(ts, TS_STATUS_CREATED);
                }
                else {
                    return ts_priv_txt_response(ts, TS_STATUS_NOT_FOUND);
                }
            }
            else {
                return ts_priv_txt_response(ts, TS_STATUS_NOT_IMPLEMENTED);
            }
        }
        else {
            return ts_priv_txt_response(ts, TS_STATUS_INTERNAL_SERVER_ERR);
        }
    }
    else if (node->type == TS_T_PUBSUB) {
        if (ts->tokens[0].type == JSMN_STRING) {
            TsDataNode *del_node = ts_get_node_by_name(ts, ts->json_str + ts->tokens[0].start,
                ts->tokens[0].end - ts->tokens[0].start, -1);
            if (del_node != NULL) {
                del_node->pubsub |= (uint16_t)node->detail;
                return ts_priv_txt_response(ts, TS_STATUS_CREATED);
            }
            return ts_priv_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
    }
    return ts_priv_txt_response(ts, TS_STATUS_METHOD_NOT_ALLOWED);
}

int ts_priv_txt_delete(ts_object_t *ts, const TsDataNode *node)
{
    if (ts->tok_count > 1) {
        // only single JSON primitive supported at the moment
        return ts_priv_txt_response(ts, TS_STATUS_NOT_IMPLEMENTED);
    }

    if (node->type == TS_T_ARRAY) {
        TsArrayInfo *arr_info = (TsArrayInfo *)node->data;
        if (arr_info->type == TS_T_NODE_ID && ts->tokens[0].type == JSMN_STRING) {
            const TsDataNode *del_node = ts_get_node_by_name(ts, ts->json_str + ts->tokens[0].start,
                ts->tokens[0].end - ts->tokens[0].start, -1);
            if (del_node != NULL) {
                // node found in node database, now look for same ID in the array
                ts_node_id_t *node_ids = (ts_node_id_t *)arr_info->ptr;
                for (int i = 0; i < arr_info->num_elements; i++) {
                    if (node_ids[i] == del_node->id) {
                        // node also found in array, shift all remaining elements
                        for (int j = i; j < arr_info->num_elements - 1; j++) {
                            node_ids[j] = node_ids[j+1];
                        }
                        arr_info->num_elements--;
                        return ts_priv_txt_response(ts, TS_STATUS_DELETED);
                    }
                }
            }
            return ts_priv_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
        else {
            return ts_priv_txt_response(ts, TS_STATUS_NOT_IMPLEMENTED);
        }
    }
    else if (node->type == TS_T_PUBSUB) {
        if (ts->tokens[0].type == JSMN_STRING) {
            TsDataNode *del_node = ts_get_node_by_name(ts, ts->json_str + ts->tokens[0].start,
                ts->tokens[0].end - ts->tokens[0].start, -1);
            if (del_node != NULL) {
                del_node->pubsub &= ~((uint16_t)node->detail);
                return ts_priv_txt_response(ts, TS_STATUS_DELETED);
            }
            return ts_priv_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
    }
    return ts_priv_txt_response(ts, TS_STATUS_METHOD_NOT_ALLOWED);
}

int ts_priv_txt_exec(ts_object_t *ts, const TsDataNode *node)
{
    int tok = 0;            // current token
    int nodes_found = 0;    // number of child nodes found

    if (ts->tok_count > 0 && ts->tokens[tok].type == JSMN_ARRAY) {
        tok++;      // go to first element of array
    }

    if ((node->access & TS_WRITE_MASK) && (node->type == TS_T_EXEC)) {
        // node is generally executable, but are we authorized?
        if ((node->access & TS_WRITE_MASK & ts->_auth_flags) == 0) {
            return ts_priv_txt_response(ts, TS_STATUS_UNAUTHORIZED);
        }
    }
    else {
        return ts_priv_txt_response(ts, TS_STATUS_FORBIDDEN);
    }

    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].parent == node->id) {
            if (tok >= ts->tok_count) {
                // more child nodes found than parameters were passed
                return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
            }
            int res = ts_priv_json_deserialize_value(ts, ts->json_str + ts->tokens[tok].start,
                ts->tokens[tok].end - ts->tokens[tok].start, ts->tokens[tok].type, &ts->data_nodes[i]);
            if (res == 0) {
                // deserializing the value was not successful
                return ts_priv_txt_response(ts, TS_STATUS_UNSUPPORTED_FORMAT);
            }
            tok += res;
            nodes_found++;
        }
    }

    if (ts->tok_count > tok) {
        // more parameters passed than child nodes found
        return ts_priv_txt_response(ts, TS_STATUS_BAD_REQUEST);
    }

    // if we got here, finally create function pointer and call function
    void (*fun)(void) = (void(*)(void))node->data;
    fun();

    return ts_priv_txt_response(ts, TS_STATUS_VALID);
}

int ts_txt_pub(ts_object_t *ts, char *buf, size_t buf_size, const uint16_t pub_ch)
{
    unsigned int len = sprintf(buf, "# {");

    for (unsigned int i = 0; i < ts->num_nodes; i++) {
        if (ts->data_nodes[i].pubsub & pub_ch) {
            len += ts_priv_json_serialize_name_value(ts, &buf[len], buf_size - len, &ts->data_nodes[i]);
        }
        if (len >= buf_size - 1) {
            return 0;
        }
    }
    buf[len-1] = '}';    // overwrite comma

    return len;
}
