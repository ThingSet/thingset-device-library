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
    case TS_STATUS_SUCCESS:
        pos = snprintf((char *)resp, resp_size, ":%d Success.", code);
        break;
    case TS_STATUS_UNKNOWN_FUNCTION:
        pos = snprintf((char *)resp, resp_size, ":%d Unknown function.", code);
        break;
    case TS_STATUS_UNKNOWN_DATA_NODE:
        pos = snprintf((char *)resp, resp_size, ":%d Data node not found for given path.", code);
        break;
    case TS_STATUS_WRONG_FORMAT:
        pos = snprintf((char *)resp, resp_size, ":%d Wrong format.", code);
        break;
    case TS_STATUS_WRONG_TYPE:
        pos = snprintf((char *)resp, resp_size, ":%d Data type not supported.", code);
        break;
    case TS_STATUS_DEVICE_BUSY:
        pos = snprintf((char *)resp, resp_size, ":%d Device busy.", code);
        break;
    case TS_STATUS_UNAUTHORIZED:
        pos = snprintf((char *)resp, resp_size, ":%d Unauthorized.", code);
        break;
    case TS_STATUS_REQUEST_TOO_LONG:
        pos = snprintf((char *)resp, resp_size, ":%d Request too long.", code);
        break;
    case TS_STATUS_RESPONSE_TOO_LONG:
        pos = snprintf((char *)resp, resp_size, ":%d Response too long.", code);
        break;
    case TS_STATUS_INVALID_VALUE:
        pos = snprintf((char *)resp, resp_size, ":%d Invalid or too large value.", code);
        break;
    case TS_STATUS_UNSUPPORTED:
        pos = snprintf((char *)resp, resp_size, ":%d Unsupported request.", code);
        break;
    case TS_STATUS_WRONG_PASSWORD:
        pos = snprintf((char *)resp, resp_size, ":%d Wrong password.", code);
        break;
    default:
        pos = snprintf((char *)resp, resp_size, ":%d Error.", code);
        break;
    };
#else
    pos = snprintf((char *)resp, resp_size, ":%d.", code);
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

int ThingSet::access_json(int function, size_t len_function)
{
    if (req_len > len_function) {
        jsmn_parser parser;
        jsmn_init(&(parser));

        // +1 because blank is requested between function and JSON data
        json_str = (char *)req + len_function + 1;
        tok_count = jsmn_parse(&parser, json_str, req_len - (len_function + 1), tokens,
            sizeof(tokens));

        if (tok_count == JSMN_ERROR_NOMEM) {
            return status_message_json(TS_STATUS_REQUEST_TOO_LONG);
        }
        else if (tok_count < 0) {
            return status_message_json(TS_STATUS_WRONG_FORMAT);
        }
        else if (tok_count == 0) {
            //printf("list_json: %s\n", json_str);
            if ((char)req[len_function-1] == '/') {
                return list_json(function, false);
            }
            else {
                return list_json(function, true);
            }
        }
        else if (tok_count == 1 && tokens[0].type == JSMN_OBJECT) {
            return list_json(function, true);
        }
        else {
            if (tokens[0].type == JSMN_OBJECT) {
                //printf("write_json: %s\n", json_str);
                int len = write_json(function);
                if (strncmp((char *)resp, ":0", 2) == 0 && conf_callback != NULL &&
                    (function == TS_CONF || function == TS_INFO)) {
                    conf_callback();
                }
                return len;
            }
            else {
                if (function == TS_EXEC) {
                    //printf("exec_json: %s\n", json_str);
                    return exec_json();
                }
                else {
                    //printf("read_json: %s\n", json_str);
                    return read_json(function);
                }
            }
        }
    }
    else {  // only function without any blank characters --> list
        if ((char)req[len_function-1] == '/') {
            return list_json(function, false);
        }
        else {
            return list_json(function, true);
        }
    }
}

int ThingSet::read_json(int category)
{
    size_t pos = 0;
    int tok = 0;       // current token

    // initialize response with success message
    pos += status_message_json(TS_STATUS_SUCCESS);

    if (tokens[0].type == JSMN_ARRAY) {
        pos += snprintf((char *)&resp[pos], resp_size - pos, " [");
        tok++;
    } else {
        pos += snprintf((char *)&resp[pos], resp_size - pos, " ");
    }

    while (tok < tok_count) {

        if (tokens[tok].type != JSMN_STRING) {
            return status_message_json(TS_STATUS_WRONG_FORMAT);
        }

        const DataNode *data_node = get_data_node(
            json_str + tokens[tok].start,
            tokens[tok].end - tokens[tok].start, category);

        if (data_node == NULL) {
            return status_message_json(TS_STATUS_UNKNOWN_DATA_NODE);
        }
        else if (data_node->type == TS_T_PATH) {
            // bad request, as we can't read internal path node's values
            return status_message_json(TS_STATUS_WRONG_FORMAT);
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
            return status_message_json(TS_STATUS_RESPONSE_TOO_LONG);
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

int ThingSet::write_json(int category)
{
    int tok = 0;       // current token

    // buffer for data node value (largest negative 64bit integer has 20 digits)
    char value_buf[21];
    size_t value_len;   // length of value in buffer

    if (tok_count < 2) {
        if (tok_count == JSMN_ERROR_NOMEM) {
            return status_message_json(TS_STATUS_REQUEST_TOO_LONG);
        } else {
            return status_message_json(TS_STATUS_WRONG_FORMAT);
        }
    }

    if (tokens[0].type == JSMN_OBJECT) {    // object = map
        tok++;
    }

    // loop through all elements to check if request is valid
    while (tok + 1 < tok_count) {

        if (tokens[tok].type != JSMN_STRING ||
            (tokens[tok+1].type != JSMN_PRIMITIVE && tokens[tok+1].type != JSMN_STRING)) {
            return status_message_json(TS_STATUS_WRONG_FORMAT);
        }

        const DataNode* data_node = get_data_node(
            json_str + tokens[tok].start,
            tokens[tok].end - tokens[tok].start, category);

        if (data_node == NULL) {
            return status_message_json(TS_STATUS_UNKNOWN_DATA_NODE);
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
            return status_message_json(TS_STATUS_INVALID_VALUE);
        } else {
            strncpy(value_buf, &json_str[tokens[tok+1].start], value_len);
            value_buf[value_len] = '\0';
        }

        errno = 0;
        if (data_node->type == TS_T_STRING) {
            if (tokens[tok+1].type != JSMN_STRING) {
                return status_message_json(TS_STATUS_WRONG_TYPE);
            }
            // data node buffer length already checked above
        }
        else if (data_node->type == TS_T_BOOL) {
            if (!(value_buf[0] == 't' || value_buf[0] == '1' || value_buf[0] == 'f' ||
                value_buf[0] == '0'))
            {
                return status_message_json(TS_STATUS_WRONG_TYPE);
            }
        }
        else {
            if (tokens[tok+1].type != JSMN_PRIMITIVE) {
                return status_message_json(TS_STATUS_WRONG_TYPE);
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
                return status_message_json(TS_STATUS_INVALID_VALUE);
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
            tokens[tok].end - tokens[tok].start, category);

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

    return status_message_json(TS_STATUS_SUCCESS);
}

int ThingSet::list_json(int category, bool values)
{
    // initialize response with success message
    size_t len = status_message_json(TS_STATUS_SUCCESS);

    len += sprintf((char *)&resp[len], values ? " {" : " [");

    for (unsigned int i = 0; i < num_nodes; i++) {
        if ((data_nodes[i].access & TS_READ_ALL) &&
            (data_nodes[i].parent == category))
        {
            if (values) {
                if (data_nodes[i].type == TS_T_PATH) {
                    // bad request, as we can't read internal path node's values
                    return status_message_json(TS_STATUS_WRONG_FORMAT);
                }
                len += json_serialize_name_value((char *)&resp[len], resp_size - len,
                    &data_nodes[i]);
            }
            else {
                len += snprintf((char *)&resp[len],
                    resp_size - len,
                    "\"%s\",", data_nodes[i].name);
            }

            if (len >= resp_size - 1) {
                return status_message_json(TS_STATUS_RESPONSE_TOO_LONG);
            }
        }
    }

    // remove trailing comma and add closing bracket
    resp[len-1] = values ? '}' : ']';

    return len;
}

int ThingSet::exec_json()
{
    if (tokens[0].type != JSMN_STRING) {
        return status_message_json(TS_STATUS_WRONG_FORMAT);
    }

    const DataNode *data_node = get_data_node(json_str + tokens[0].start,
        tokens[0].end - tokens[0].start, TS_EXEC);
    if (data_node == NULL) {
        return status_message_json(TS_STATUS_UNKNOWN_DATA_NODE);
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

    return status_message_json(TS_STATUS_SUCCESS);
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
    const int len_function = 5; // !auth
    jsmn_parser parser;
    jsmn_init(&(parser));

    json_str = (char *)req + len_function + 1;
    tok_count = jsmn_parse(&parser, json_str, req_len - (len_function + 1), tokens, sizeof(tokens));

    if (req_len == len_function || tok_count == 0) {   // logout
        user_authorized = false;
        maker_authorized = false;
        return status_message_json(TS_STATUS_SUCCESS);
    }
    else if (tok_count == 1) {
        if (tokens[0].type != JSMN_STRING) {
            return status_message_json(TS_STATUS_WRONG_FORMAT);
        }

        if (user_pass != NULL &&
            tokens[0].end - tokens[0].start == (int)strlen(user_pass) &&
            strncmp(json_str + tokens[0].start, user_pass, strlen(user_pass)) == 0)
        {
            user_authorized = true;
            return status_message_json(TS_STATUS_SUCCESS);
        }
        else if (maker_pass != NULL &&
            tokens[0].end - tokens[0].start == (int)strlen(maker_pass) &&
            strncmp(json_str + tokens[0].start, maker_pass, strlen(maker_pass)) == 0)
        {
            user_authorized = true;     // authorize maker also for user access
            maker_authorized = true;
            return status_message_json(TS_STATUS_SUCCESS);
        }
        else {
            user_authorized = false;
            maker_authorized = false;
            return status_message_json(TS_STATUS_WRONG_PASSWORD);
        }
    }
    else {
        return status_message_json(TS_STATUS_WRONG_FORMAT);
    }

    return 0;
}

int ThingSet::pub_json()
{
    const int len_function = 4; // !pub
    jsmn_parser parser;
    jsmn_init(&(parser));

    json_str = (char *)req + len_function + 1;
    tok_count = jsmn_parse(&parser, json_str, req_len - (len_function + 1), tokens, sizeof(tokens));

    // initialize response with success message
    size_t len = status_message_json(TS_STATUS_SUCCESS);

    if (req_len == len_function || tok_count == 0) {
        // list channels
        len += sprintf((char *)&resp[len], " [");
        for (unsigned int i = 0; i < num_channels; i++) {
            len += snprintf((char *)&resp[len], resp_size - len, "\"%s\",", pub_channels[i].name);
            if (len >= resp_size - 1) {
                return status_message_json(TS_STATUS_RESPONSE_TOO_LONG);
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
        return status_message_json(TS_STATUS_UNSUPPORTED);
    }
    else if (tok_count >= 3) {   // map with at least one key/value pair
        // change channel setting
        if (tokens[0].type != JSMN_OBJECT && tokens[1].type != JSMN_STRING) {
            return status_message_json(TS_STATUS_WRONG_FORMAT);
        }

        ts_pub_channel_t* pub_ch = get_pub_channel(
            json_str + tokens[1].start,
            tokens[1].end - tokens[1].start);

        if (pub_ch == NULL) {
            return status_message_json(TS_STATUS_UNKNOWN_DATA_NODE);
        }

        if (tokens[2].type == JSMN_PRIMITIVE) {
            if (json_str[tokens[2].start] == 't' || json_str[tokens[2].start] == '1') {
                pub_ch->enabled = true;
            }
            else if (json_str[tokens[2].start] == 'f' || json_str[tokens[2].start] == '0') {
                pub_ch->enabled = false;
            }
            else {
                return status_message_json(TS_STATUS_WRONG_FORMAT);
            }
            return status_message_json(TS_STATUS_SUCCESS);
        }
        return status_message_json(TS_STATUS_WRONG_FORMAT);
    }
    else {
        return status_message_json(TS_STATUS_WRONG_FORMAT);
    }
}
