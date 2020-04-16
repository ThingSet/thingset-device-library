/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#include "ts_config.h"
#include "thingset.h"
#include "jsmn.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <cinttypes>


int ThingSet::status_message_json(int code)
{
    size_t pos = 0;
#ifdef TS_VERBOSE_STATUS_MESSAGES
    switch(code) {
        // success
        case TS_STATUS_CREATED:
            pos = snprintf((char *)resp, resp_size, ":%.2X Created.", code);
            break;
        case TS_STATUS_DELETED:
            pos = snprintf((char *)resp, resp_size, ":%.2X Deleted.", code);
            break;
        case TS_STATUS_VALID:
            pos = snprintf((char *)resp, resp_size, ":%.2X Valid.", code);
            break;
        case TS_STATUS_CHANGED:
            pos = snprintf((char *)resp, resp_size, ":%.2X Changed.", code);
            break;
        case TS_STATUS_CONTENT:
            pos = snprintf((char *)resp, resp_size, ":%.2X Content.", code);
            break;
        // client errors
        case TS_STATUS_BAD_REQUEST:
            pos = snprintf((char *)resp, resp_size, ":%.2X Bad Request.", code);
            break;
        case TS_STATUS_UNAUTHORIZED:
            pos = snprintf((char *)resp, resp_size, ":%.2X Unauthorized.", code);
            break;
        case TS_STATUS_FORBIDDEN:
            pos = snprintf((char *)resp, resp_size, ":%.2X Forbidden.", code);
            break;
        case TS_STATUS_NOT_FOUND:
            pos = snprintf((char *)resp, resp_size, ":%.2X Not Found.", code);
            break;
        case TS_STATUS_METHOD_NOT_ALLOWED:
            pos = snprintf((char *)resp, resp_size, ":%.2X Method Not Allowed.", code);
            break;
        case TS_STATUS_REQUEST_INCOMPLETE:
            pos = snprintf((char *)resp, resp_size, ":%.2X Request Entity Incomplete.", code);
            break;
        case TS_STATUS_CONFLICT:
            pos = snprintf((char *)resp, resp_size, ":%.2X Conflict.", code);
            break;
        case TS_STATUS_REQUEST_TOO_LARGE:
            pos = snprintf((char *)resp, resp_size, ":%.2X Request Entity Too Large.", code);
            break;
        case TS_STATUS_UNSUPPORTED_FORMAT:
            pos = snprintf((char *)resp, resp_size, ":%.2X Unsupported Content-Format.", code);
            break;
        // server errors
        case TS_STATUS_INTERNAL_SERVER_ERR:
            pos = snprintf((char *)resp, resp_size, ":%.2X Internal Server Error.", code);
            break;
        case TS_STATUS_NOT_IMPLEMENTED:
            pos = snprintf((char *)resp, resp_size, ":%.2X Not Implemented.", code);
            break;
        default:
            pos = snprintf((char *)resp, resp_size, ":%.2X Error.", code);
            break;
    };
#else
    pos = snprintf((char *)resp, resp_size, ":%.2X.", code);
#endif
    if (pos < resp_size)
        return pos;
    else
        return 0;
}

int ThingSet::json_serialize_value(char *resp, size_t size, const DataNode *data_node)
{
    size_t pos = 0;
    const DataNode *sub_node;

    switch (data_node->type) {
#ifdef TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        pos = snprintf(&resp[pos], size - pos, "%" PRIu64 ",", *((uint64_t *)data_node->data));
        break;
    case TS_T_INT64:
        pos = snprintf(&resp[pos], size - pos, "%" PRIi64 ",", *((int64_t *)data_node->data));
        break;
#endif
    case TS_T_UINT32:
        pos = snprintf(&resp[pos], size - pos, "%" PRIu32 ",", *((uint32_t *)data_node->data));
        break;
    case TS_T_INT32:
        pos = snprintf(&resp[pos], size - pos, "%" PRIi32 ",", *((int32_t *)data_node->data));
        break;
    case TS_T_UINT16:
        pos = snprintf(&resp[pos], size - pos, "%" PRIu16 ",", *((uint16_t *)data_node->data));
        break;
    case TS_T_INT16:
        pos = snprintf(&resp[pos], size - pos, "%" PRIi16 ",", *((int16_t *)data_node->data));
        break;
    case TS_T_FLOAT32:
        pos = snprintf(&resp[pos], size - pos, "%.*f,", data_node->detail,
                *((float *)data_node->data));
        break;
    case TS_T_BOOL:
        pos = snprintf(&resp[pos], size - pos, "%s,",
                (*((bool *)data_node->data) == true ? "true" : "false"));
        break;
    case TS_T_FUNCTION:
        pos = snprintf(&resp[pos], size - pos, "null,");
        break;
    case TS_T_STRING:
        pos = snprintf(&resp[pos], size - pos, "\"%s\",", (char *)data_node->data);
        break;
    case TS_T_ARRAY:
        ArrayInfo *array_info = (ArrayInfo *)data_node->data;
        if (!array_info) {
            return 0;
        }
        pos += snprintf(&resp[pos], size - pos, "[");
        for (int i = 0; i < array_info->num_elements; i++) {
            switch (array_info->type) {
            case TS_T_UINT64:
                pos += snprintf(&resp[pos], size - pos, "%" PRIu64 ",",
                        ((uint64_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT64:
                pos += snprintf(&resp[pos], size - pos, "%" PRIi64 ",",
                        ((int64_t *)array_info->ptr)[i]);
                break;
            case TS_T_UINT32:
                pos += snprintf(&resp[pos], size - pos, "%" PRIu32 ",",
                        ((uint32_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT32:
                pos += snprintf(&resp[pos], size - pos, "%" PRIi32 ",",
                        ((int32_t *)array_info->ptr)[i]);
                break;
            case TS_T_UINT16:
                pos += snprintf(&resp[pos], size - pos, "%" PRIu16 ",",
                        ((uint16_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT16:
                pos += snprintf(&resp[pos], size - pos, "%" PRIi16 ",",
                        ((int16_t *)array_info->ptr)[i]);
                break;
            case TS_T_FLOAT32:
                pos += snprintf(&resp[pos], size - pos, "%.*f,", data_node->detail,
                        ((float *)array_info->ptr)[i]);
                break;
            case TS_T_NODE_ID:
                sub_node = get_data_node(((uint16_t *)array_info->ptr)[i]);
                if (sub_node) {
                    pos += snprintf(&resp[pos], size - pos, "\"%s\",", sub_node->name);
                }
                break;
            default:
                break;
            }
        }
        pos--; // remove trailing comma
        pos += snprintf(&resp[pos], size - pos, "],");
        break;
    }

    if (pos < size) {
        return pos;
    }
    else {
        return 0;
    }
}

int ThingSet::json_serialize_name_value(char *resp, size_t size, const DataNode* data_node)
{
    size_t pos = snprintf(resp, size, "\"%s\":", data_node->name);

    if (pos < size) {
        return pos + json_serialize_value(&resp[pos], size - pos, data_node);
    }
    else {
        return 0;
    }
}

void ThingSet::dump_json(uint16_t node_id, int level)
{
    uint8_t buf[100];
    bool first = true;
    for (int i = 0; i < num_nodes; i++) {
        if (data_nodes[i].parent == node_id) {
            if (!first) {
                printf(",\n");
            }
            else {
                printf("\n");
                first = false;
            }
            if (data_nodes[i].type == TS_T_PATH) {
                printf("%*s\"%s\" {", 4 * level, "", data_nodes[i].name);
                dump_json(data_nodes[i].id, level + 1);
                printf("\n%*s}", 4 * level, "");
            }
            else {
                int pos = json_serialize_name_value((char *)buf, sizeof(buf), &data_nodes[i]);
                if (pos > 0) {
                    buf[pos-1] = '\0';  // remove trailing comma
                    printf("%*s%s", 4 * level, "", (char *)buf);
                }
            }
        }
    }
    if (node_id == 0) {
        printf("\n");
    }
}

int ThingSet::process_json()
{
    int path_len = req_len;
    char *path_end = strchr((char *)req + 1, ' ');
    if (path_end) {
        path_len = (uint8_t *)path_end - req;
    }

    const DataNode *endpoint = get_endpoint_node((char *)req + 1, path_len - 1);
    if (!endpoint) {
        return status_message_json(TS_STATUS_NOT_FOUND);
    }

    jsmn_parser parser;
    jsmn_init(&parser);

    json_str = (char *)req + path_len;
    tok_count = jsmn_parse(&parser, json_str, req_len - path_len, tokens, sizeof(tokens));

    if (tok_count == JSMN_ERROR_NOMEM) {
        return status_message_json(TS_STATUS_REQUEST_TOO_LARGE);
    }
    else if (tok_count < 0) {
        // other parsing error
        return status_message_json(TS_STATUS_BAD_REQUEST);
    }
    else if (tok_count == 0) {
        // no payload data
        if ((char)req[path_len-1] == '/') {
            return get_json(endpoint, false);
        }
        else {
            return get_json(endpoint, true);
        }
    }
    else if (tok_count == 1 && tokens[0].type == JSMN_OBJECT) {
        // legacy function to get all sub-nodes of a node by passing empty map
        return get_json(endpoint, true);
    }
    else {
        if (tokens[0].type == JSMN_OBJECT) {
            //printf("patch_json: %s\n", json_str);
            int len = patch_json(endpoint->id);
            if (strncmp((char *)resp, ":84", 3) == 0 && conf_callback != NULL &&
                (endpoint->id == TS_CONF || endpoint->id == TS_INFO)) {
                conf_callback();
            }
            return len;
        }
        else {
            if (endpoint->id == TS_EXEC) {
                //printf("exec_json: %s\n", json_str);
                return exec_json();
            }
            else {
                //printf("fetch_json: %s\n", json_str);
                return fetch_json(endpoint->id);
            }
        }
    }
}

int ThingSet::fetch_json(uint16_t parent_id)
{
    size_t pos = 0;
    int tok = 0;       // current token

    // initialize response with success message
    pos += status_message_json(TS_STATUS_CONTENT);

    if (tokens[0].type == JSMN_ARRAY) {
        pos += snprintf((char *)&resp[pos], resp_size - pos, " [");
        tok++;
    } else {
        pos += snprintf((char *)&resp[pos], resp_size - pos, " ");
    }

    while (tok < tok_count) {

        if (tokens[tok].type != JSMN_STRING) {
            return status_message_json(TS_STATUS_BAD_REQUEST);
        }

        const DataNode *data_node = get_data_node(
            json_str + tokens[tok].start,
            tokens[tok].end - tokens[tok].start, parent_id);

        if (data_node == NULL) {
            return status_message_json(TS_STATUS_NOT_FOUND);
        }
        else if (data_node->type == TS_T_PATH) {
            // bad request, as we can't read internal path node's values
            return status_message_json(TS_STATUS_BAD_REQUEST);
        }
        if (!( ((data_node->access & TS_READ_MASK) == TS_READ_ALL) ||
               (((data_node->access & TS_READ_MASK) == TS_READ_USER) && user_authorized) ||
               (((data_node->access & TS_READ_MASK) == TS_READ_MAKER) && maker_authorized)
           ))
        {
            return status_message_json(TS_STATUS_UNAUTHORIZED);
        }

        pos += json_serialize_value((char *)&resp[pos], resp_size - pos, data_node);

        if (pos >= resp_size - 2) {
            return status_message_json(TS_STATUS_RESPONSE_TOO_LARGE);
        }
        tok++;
    }

    pos--;  // remove trailing comma
    if (tokens[0].type == JSMN_ARRAY) {
        // buffer will be long enough as we dropped last 2 characters --> sprintf allowed
        pos += sprintf((char *)&resp[pos], "]");
    } else {
        resp[pos] = '\0';    // terminate string
    }

    return pos;
}

int ThingSet::patch_json(uint16_t parent_id)
{
    int tok = 0;       // current token

    // buffer for data node value (largest negative 64bit integer has 20 digits)
    char value_buf[21];
    size_t value_len;   // length of value in buffer

    if (tok_count < 2) {
        if (tok_count == JSMN_ERROR_NOMEM) {
            return status_message_json(TS_STATUS_REQUEST_TOO_LARGE);
        } else {
            return status_message_json(TS_STATUS_BAD_REQUEST);
        }
    }

    if (tokens[0].type == JSMN_OBJECT) {    // object = map
        tok++;
    }

    // loop through all elements to check if request is valid
    while (tok + 1 < tok_count) {

        if (tokens[tok].type != JSMN_STRING ||
            (tokens[tok+1].type != JSMN_PRIMITIVE && tokens[tok+1].type != JSMN_STRING)) {
            return status_message_json(TS_STATUS_BAD_REQUEST);
        }

        const DataNode* data_node = get_data_node(
            json_str + tokens[tok].start,
            tokens[tok].end - tokens[tok].start, parent_id);

        if (data_node == NULL) {
            return status_message_json(TS_STATUS_NOT_FOUND);
        }

        if (!( ((data_node->access & TS_WRITE_MASK) == TS_WRITE_ALL) ||
               (((data_node->access & TS_WRITE_MASK) == TS_WRITE_USER) && user_authorized) ||
               (((data_node->access & TS_WRITE_MASK) == TS_WRITE_MAKER) && maker_authorized)
           ))
        {
            return status_message_json(TS_STATUS_UNAUTHORIZED);
        }

        // extract the value and check buffer lengths
        value_len = tokens[tok+1].end - tokens[tok+1].start;
        if ((data_node->type != TS_T_STRING && value_len >= sizeof(value_buf)) ||
            (data_node->type == TS_T_STRING && value_len >= (size_t)data_node->detail)) {
            return status_message_json(TS_STATUS_UNSUPPORTED_FORMAT);
        } else {
            strncpy(value_buf, &json_str[tokens[tok+1].start], value_len);
            value_buf[value_len] = '\0';
        }

        errno = 0;
        if (data_node->type == TS_T_STRING) {
            if (tokens[tok+1].type != JSMN_STRING) {
                return status_message_json(TS_STATUS_UNSUPPORTED_FORMAT);
            }
            // data node buffer length already checked above
        }
        else if (data_node->type == TS_T_BOOL) {
            if (!(value_buf[0] == 't' || value_buf[0] == '1' || value_buf[0] == 'f' ||
                value_buf[0] == '0'))
            {
                return status_message_json(TS_STATUS_UNSUPPORTED_FORMAT);
            }
        }
        else {
            if (tokens[tok+1].type != JSMN_PRIMITIVE) {
                return status_message_json(TS_STATUS_UNSUPPORTED_FORMAT);
            }
            if (data_node->type == TS_T_FLOAT32) {
                strtod(value_buf, NULL);
            }
            else if (data_node->type == TS_T_UINT32 || data_node->type == TS_T_UINT16) {
                strtoul(value_buf, NULL, 0);
            }
            else if (data_node->type == TS_T_INT32 || data_node->type == TS_T_INT16) {
                strtol(value_buf, NULL, 0);
            }
            else if (data_node->type == TS_T_UINT64) {
                strtoull(value_buf, NULL, 0);
            }
            else if (data_node->type == TS_T_INT64) {
                strtoll(value_buf, NULL, 0);
            }

            if (errno == ERANGE) {
                return status_message_json(TS_STATUS_UNSUPPORTED_FORMAT);
            }
        }
        tok += 2;   // map expected --> always one string + one value
    }

    if (tokens[0].type == JSMN_OBJECT)
        tok = 1;
    else
        tok = 0;

    // actually write data
    while (tok + 1 < tok_count) {

        const DataNode *data_node = get_data_node(
            json_str + tokens[tok].start,
            tokens[tok].end - tokens[tok].start, parent_id);

        // extract the value again (max. size was checked before)
        value_len = tokens[tok+1].end - tokens[tok+1].start;
        if (value_len < sizeof(value_buf)) {
            strncpy(value_buf, &json_str[tokens[tok+1].start], value_len);
            value_buf[value_len] = '\0';
        }

        switch (data_node->type) {
        case TS_T_FLOAT32:
            *((float*)data_node->data) = strtod(value_buf, NULL);
            break;
        case TS_T_UINT64:
            *((uint64_t*)data_node->data) = strtoull(value_buf, NULL, 0);
            break;
        case TS_T_INT64:
            *((int64_t*)data_node->data) = strtoll(value_buf, NULL, 0);
            break;
        case TS_T_UINT32:
            *((uint32_t*)data_node->data) = strtoul(value_buf, NULL, 0);
            break;
        case TS_T_INT32:
            *((int32_t*)data_node->data) = strtol(value_buf, NULL, 0);
            break;
        case TS_T_UINT16:
            *((uint16_t*)data_node->data) = strtoul(value_buf, NULL, 0);
            break;
        case TS_T_INT16:
            *((uint16_t*)data_node->data) = strtol(value_buf, NULL, 0);
            break;
        case TS_T_BOOL:
            if (value_buf[0] == 't' || value_buf[0] == '1') {
                *((bool*)data_node->data) = true;
            }
            else if (value_buf[0] == 'f' || value_buf[0] == '0') {
                *((bool*)data_node->data) = false;
            }
            break;
        case TS_T_STRING:
            strncpy((char*)data_node->data, &json_str[tokens[tok+1].start], value_len);
            ((char*)data_node->data)[value_len] = '\0';
            break;
        }
        tok += 2;   // map expected --> always one string + one value
    }

    return status_message_json(TS_STATUS_CHANGED);
}

int ThingSet::get_json(const DataNode *parent_node, bool include_values)
{
    // initialize response with success message
    size_t len = status_message_json(TS_STATUS_CONTENT);

    if (parent_node->type != TS_T_PATH) {
        // get value of data node
        resp[len++] = ' ';
        len += json_serialize_value((char *)&resp[len], resp_size - len, parent_node);
        resp[--len] = '\0';     // remove trailing comma again
        return len;
    }

    len += sprintf((char *)&resp[len], include_values ? " {" : " [");
    int nodes_found = 0;
    for (unsigned int i = 0; i < num_nodes; i++) {
        if ((data_nodes[i].access & TS_READ_ALL) &&
            (data_nodes[i].parent == parent_node->id))
        {
            if (include_values) {
                if (data_nodes[i].type == TS_T_PATH) {
                    // bad request, as we can't read internal path node's values
                    return status_message_json(TS_STATUS_BAD_REQUEST);
                }
                len += json_serialize_name_value((char *)&resp[len], resp_size - len,
                    &data_nodes[i]);
            }
            else {
                len += snprintf((char *)&resp[len],
                    resp_size - len,
                    "\"%s\",", data_nodes[i].name);
            }
            nodes_found++;

            if (len >= resp_size - 1) {
                return status_message_json(TS_STATUS_RESPONSE_TOO_LARGE);
            }
        }
    }

    // remove trailing comma and add closing bracket
    if (nodes_found == 0) {
        len++;
    }
    resp[len-1] = include_values ? '}' : ']';
    resp[len] = '\0';

    return len;
}

int ThingSet::exec_json()
{
    if (tokens[0].type != JSMN_STRING) {
        return status_message_json(TS_STATUS_BAD_REQUEST);
    }

    const DataNode *data_node = get_data_node(json_str + tokens[0].start,
        tokens[0].end - tokens[0].start, TS_EXEC);
    if (data_node == NULL) {
        return status_message_json(TS_STATUS_NOT_FOUND);
    }
    if (!( ((data_node->access & TS_EXEC_MASK) == TS_EXEC_ALL) ||
            (((data_node->access & TS_EXEC_MASK) == TS_EXEC_USER) && user_authorized) ||
            (((data_node->access & TS_EXEC_MASK) == TS_EXEC_MAKER) && maker_authorized)
        ))
    {
        return status_message_json(TS_STATUS_UNAUTHORIZED);
    }

    // create function pointer and call function
    void (*fun)(void) = reinterpret_cast<void(*)()>(data_node->data);
    fun();

    return status_message_json(TS_STATUS_VALID);
}

int ThingSet::pub_msg_json(char *msg_buf, size_t size, unsigned int channel)
{
    unsigned int len = sprintf(msg_buf, "# {");

    if (num_channels < channel) {   // unknown channel
        return 0;
    }

    for (unsigned int element = 0; element < pub_channels[channel].num; element++) {

        const DataNode* data_node = get_data_node(pub_channels[channel].object_ids[element]);
        if (data_node == NULL || !(data_node->access & TS_READ_ALL)) {
            continue;
        }

        len += json_serialize_name_value(&msg_buf[len], size - len, data_node);

        if (len >= size - 1) {
            return 0;
        }
    }
    msg_buf[len-1] = '}';    // overwrite comma

    return len;
}

int ThingSet::auth_json()
{
    const int path_len = 5; // !auth
    jsmn_parser parser;
    jsmn_init(&(parser));

    json_str = (char *)req + path_len + 1;
    tok_count = jsmn_parse(&parser, json_str, req_len - (path_len + 1), tokens, sizeof(tokens));

    if (req_len == path_len || tok_count == 0) {   // logout
        user_authorized = false;
        maker_authorized = false;
        return status_message_json(TS_STATUS_VALID);
    }
    else if (tok_count == 1) {
        if (tokens[0].type != JSMN_STRING) {
            return status_message_json(TS_STATUS_BAD_REQUEST);
        }

        if (user_pass != NULL &&
            tokens[0].end - tokens[0].start == (int)strlen(user_pass) &&
            strncmp(json_str + tokens[0].start, user_pass, strlen(user_pass)) == 0)
        {
            user_authorized = true;
            return status_message_json(TS_STATUS_VALID);
        }
        else if (maker_pass != NULL &&
            tokens[0].end - tokens[0].start == (int)strlen(maker_pass) &&
            strncmp(json_str + tokens[0].start, maker_pass, strlen(maker_pass)) == 0)
        {
            user_authorized = true;     // authorize maker also for user access
            maker_authorized = true;
            return status_message_json(TS_STATUS_VALID);
        }
        else {
            user_authorized = false;
            maker_authorized = false;
            return status_message_json(TS_STATUS_CONFLICT);
        }
    }
    else {
        return status_message_json(TS_STATUS_BAD_REQUEST);
    }

    return 0;
}

int ThingSet::pub_json()
{
    const int path_len = 4; // !pub
    jsmn_parser parser;
    jsmn_init(&(parser));

    json_str = (char *)req + path_len + 1;
    tok_count = jsmn_parse(&parser, json_str, req_len - (path_len + 1), tokens, sizeof(tokens));

    // initialize response with success message
    size_t len = status_message_json(TS_STATUS_CONTENT);

    if (req_len == path_len || tok_count == 0) {
        // list channels
        len += sprintf((char *)&resp[len], " [");
        for (unsigned int i = 0; i < num_channels; i++) {
            len += snprintf((char *)&resp[len], resp_size - len, "\"%s\",", pub_channels[i].name);
            if (len >= resp_size - 1) {
                return status_message_json(TS_STATUS_RESPONSE_TOO_LARGE);
            }
        }
        // remove trailing comma and add closing bracket
        resp[len-1] = ']';
        return len;
    }
    else if (tok_count == 1) {
        // can be:
        // - true/false to globally enable/disable publication messages
        // - a string to list elements of that channel
        return status_message_json(TS_STATUS_UNSUPPORTED_FORMAT);
    }
    else if (tok_count >= 3) {   // map with at least one key/value pair
        // change channel setting
        if (tokens[0].type != JSMN_OBJECT && tokens[1].type != JSMN_STRING) {
            return status_message_json(TS_STATUS_BAD_REQUEST);
        }

        ts_pub_channel_t* pub_ch = get_pub_channel(
            json_str + tokens[1].start,
            tokens[1].end - tokens[1].start);

        if (pub_ch == NULL) {
            return status_message_json(TS_STATUS_NOT_FOUND);
        }

        if (tokens[2].type == JSMN_PRIMITIVE) {
            if (json_str[tokens[2].start] == 't' || json_str[tokens[2].start] == '1') {
                pub_ch->enabled = true;
            }
            else if (json_str[tokens[2].start] == 'f' || json_str[tokens[2].start] == '0') {
                pub_ch->enabled = false;
            }
            else {
                return status_message_json(TS_STATUS_BAD_REQUEST);
            }
            return status_message_json(TS_STATUS_CHANGED);
        }
        return status_message_json(TS_STATUS_BAD_REQUEST);
    }
    else {
        return status_message_json(TS_STATUS_BAD_REQUEST);
    }
}
