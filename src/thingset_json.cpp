/* ThingSet protocol client library
 * Copyright (c) 2017-2019 Martin Jäger (www.libre.solar)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
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
    case TS_STATUS_UNKNOWN_DATA_OBJ:
        pos = snprintf((char *)resp, resp_size, ":%d Data object not found.", code);
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
    case TS_STATUS_WRONG_CATEGORY:
        pos = snprintf((char *)resp, resp_size, ":%d Wrong category.", code);
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

static int json_serialize_value(char *resp, size_t size, const data_object_t* data_obj)
{
    size_t pos = 0;

    switch (data_obj->type) {
#ifdef TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        pos = snprintf(&resp[pos], size - pos, "%" PRIu64 ",", *((uint64_t*)data_obj->data));
        break;
    case TS_T_INT64:
        pos = snprintf(&resp[pos], size - pos, "%" PRIi64 ",", *((int64_t*)data_obj->data));
        break;
#endif
    case TS_T_UINT32:
        pos = snprintf(&resp[pos], size - pos, "%" PRIu32 ",", *((uint32_t*)data_obj->data));
        break;
    case TS_T_INT32:
        pos = snprintf(&resp[pos], size - pos, "%" PRIi32 ",", *((int32_t*)data_obj->data));
        break;
    case TS_T_UINT16:
        pos = snprintf(&resp[pos], size - pos, "%" PRIu16 ",", *((uint16_t*)data_obj->data));
        break;
    case TS_T_INT16:
        pos = snprintf(&resp[pos], size - pos, "%" PRIi16 ",", *((int16_t*)data_obj->data));
        break;
    case TS_T_FLOAT32:
        pos = snprintf(&resp[pos], size - pos, "%.*f,", data_obj->detail, *((float*)data_obj->data));
        break;
    case TS_T_BOOL:
        pos = snprintf(&resp[pos], size - pos, "%s,", (*((bool*)data_obj->data) == true ? "true" : "false"));
        break;
    case TS_T_STRING:
        pos = snprintf(&resp[pos], size - pos, "\"%s\",", (char*)data_obj->data);
        break;
    }

    if (pos < size)
        return pos;
    else
        return 0;
}

static int json_serialize_name_value(char *resp, size_t size, const data_object_t* data_obj)
{
    size_t pos = snprintf(resp, size, "\"%s\":", data_obj->name);

    if (pos < size)
        return pos + json_serialize_value(&resp[pos], size - pos, data_obj);
    else
        return 0;
}

int ThingSet::access_json(int function, size_t len_function)
{
    if (req_len > len_function) {
        jsmn_parser parser;
        jsmn_init(&(parser));

        json_str = (char *)req + len_function + 1;      // +1 because blank is requested between function and JSON data
        tok_count = jsmn_parse(&parser, json_str, req_len - (len_function + 1), tokens, sizeof(tokens));

        if (tok_count == JSMN_ERROR_NOMEM) {
            return status_message_json(TS_STATUS_REQUEST_TOO_LONG);
        }
        else if (tok_count < 0) {
            return status_message_json(TS_STATUS_WRONG_FORMAT);
        }
        else if (tok_count == 0) {
            //printf("list_json: %s\n", json_str);
            return list_json(function);
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
        return list_json(function);
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

        const data_object_t *data_obj = get_data_object(
            json_str + tokens[tok].start,
            tokens[tok].end - tokens[tok].start);

        if (data_obj == NULL) {
            return status_message_json(TS_STATUS_UNKNOWN_DATA_OBJ);
        }
        if (!( ((data_obj->access & TS_READ_MASK) == TS_READ_ALL) ||
               (((data_obj->access & TS_READ_MASK) == TS_READ_USER) && user_authorized) ||
               (((data_obj->access & TS_READ_MASK) == TS_READ_MAKER) && maker_authorized)
           ))
        {
            return status_message_json(TS_STATUS_UNAUTHORIZED);
        }

        if (!(data_obj->category & category)) {
            return status_message_json(TS_STATUS_WRONG_CATEGORY);
        }

        pos += json_serialize_value((char *)&resp[pos], resp_size - pos, data_obj);

        if (pos >= resp_size - 2) {
            return status_message_json(TS_STATUS_RESPONSE_TOO_LONG);
        }
        tok++;
    }

    pos--;  // remove trailing comma
    if (tokens[0].type == JSMN_ARRAY) {
        pos += sprintf((char *)&resp[pos], "]");     // buffer will be long enough as we dropped last 2 characters --> sprintf allowed
    } else {
        resp[pos] = '\0';    // terminate string
    }

    return pos;
}

int ThingSet::write_json(int category)
{
    int tok = 0;       // current token

    char value_buf[21];       // buffer for data object value (largest negative 64bit integer has 20 digits)
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

        const data_object_t* data_obj = get_data_object(
            json_str + tokens[tok].start,
            tokens[tok].end - tokens[tok].start);

        if (data_obj == NULL) {
            return status_message_json(TS_STATUS_UNKNOWN_DATA_OBJ);
        }

        if (!( ((data_obj->access & TS_WRITE_MASK) == TS_WRITE_ALL) ||
               (((data_obj->access & TS_WRITE_MASK) == TS_WRITE_USER) && user_authorized) ||
               (((data_obj->access & TS_WRITE_MASK) == TS_WRITE_MAKER) && maker_authorized)
           ))
        {
            return status_message_json(TS_STATUS_UNAUTHORIZED);
        }

        if (data_obj->category != category) {
            return status_message_json(TS_STATUS_WRONG_CATEGORY);
        }

        // extract the value and check buffer lengths
        value_len = tokens[tok+1].end - tokens[tok+1].start;
        if ((data_obj->type != TS_T_STRING && value_len >= sizeof(value_buf)) ||
            (data_obj->type == TS_T_STRING && value_len >= (size_t)data_obj->detail)) {
            return status_message_json(TS_STATUS_INVALID_VALUE);
        } else {
            strncpy(value_buf, &json_str[tokens[tok+1].start], value_len);
            value_buf[value_len] = '\0';
        }

        errno = 0;
        if (data_obj->type == TS_T_STRING) {
            if (tokens[tok+1].type != JSMN_STRING) {
                return status_message_json(TS_STATUS_WRONG_TYPE);
            }
            // data object buffer length already checked above
        }
        else if (data_obj->type == TS_T_BOOL) {
            if (!(value_buf[0] == 't' || value_buf[0] == '1' || value_buf[0] == 'f' || value_buf[0] == '0')) {
                return status_message_json(TS_STATUS_WRONG_TYPE);
            }
        }
        else {
            if (tokens[tok+1].type != JSMN_PRIMITIVE) {
                return status_message_json(TS_STATUS_WRONG_TYPE);
            }
            if (data_obj->type == TS_T_FLOAT32) {
                strtod(value_buf, NULL);
            }
            else if (data_obj->type == TS_T_UINT32 || data_obj->type == TS_T_UINT16) {
                strtoul(value_buf, NULL, 0);
            }
            else if (data_obj->type == TS_T_INT32 || data_obj->type == TS_T_INT16) {
                strtol(value_buf, NULL, 0);
            }
            else if (data_obj->type == TS_T_UINT64) {
                strtoull(value_buf, NULL, 0);
            }
            else if (data_obj->type == TS_T_INT64) {
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

        const data_object_t *data_obj = get_data_object(
            json_str + tokens[tok].start,
            tokens[tok].end - tokens[tok].start);

        // extract the value again (max. size was checked before)
        value_len = tokens[tok+1].end - tokens[tok+1].start;
        if (value_len < sizeof(value_buf)) {
            strncpy(value_buf, &json_str[tokens[tok+1].start], value_len);
            value_buf[value_len] = '\0';
        }

        switch (data_obj->type) {
        case TS_T_FLOAT32:
            *((float*)data_obj->data) = strtod(value_buf, NULL);
            break;
        case TS_T_UINT64:
            *((uint64_t*)data_obj->data) = strtoull(value_buf, NULL, 0);
            break;
        case TS_T_INT64:
            *((int64_t*)data_obj->data) = strtoll(value_buf, NULL, 0);
            break;
        case TS_T_UINT32:
            *((uint32_t*)data_obj->data) = strtoul(value_buf, NULL, 0);
            break;
        case TS_T_INT32:
            *((int32_t*)data_obj->data) = strtol(value_buf, NULL, 0);
            break;
        case TS_T_UINT16:
            *((uint16_t*)data_obj->data) = strtoul(value_buf, NULL, 0);
            break;
        case TS_T_INT16:
            *((uint16_t*)data_obj->data) = strtol(value_buf, NULL, 0);
            break;
        case TS_T_BOOL:
            if (value_buf[0] == 't' || value_buf[0] == '1') {
                *((bool*)data_obj->data) = true;
            }
            else if (value_buf[0] == 'f' || value_buf[0] == '0') {
                *((bool*)data_obj->data) = false;
            }
            break;
        case TS_T_STRING:
            strncpy((char*)data_obj->data, &json_str[tokens[tok+1].start], value_len);
            ((char*)data_obj->data)[value_len] = '\0';
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

    for (unsigned int i = 0; i < num_objects; i++) {
        if ((data_objects[i].access & TS_READ_ALL) &&
            (data_objects[i].category == category))
        {
            if (values) {
                len += json_serialize_name_value((char *)&resp[len], resp_size - len, &data_objects[i]);
            }
            else {
                len += snprintf((char *)&resp[len],
                    resp_size - len,
                    "\"%s\",", data_objects[i].name);
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

    const data_object_t *data_obj = get_data_object(json_str + tokens[0].start, tokens[0].end - tokens[0].start);
    if (data_obj == NULL) {
        return status_message_json(TS_STATUS_UNKNOWN_DATA_OBJ);
    }
    if (!( ((data_obj->access & TS_EXEC_MASK) == TS_EXEC_ALL) ||
            (((data_obj->access & TS_EXEC_MASK) == TS_EXEC_USER) && user_authorized) ||
            (((data_obj->access & TS_EXEC_MASK) == TS_EXEC_MAKER) && maker_authorized)
        ))
    {
        return status_message_json(TS_STATUS_UNAUTHORIZED);
    }

    // create function pointer and call function
    void (*fun)(void) = reinterpret_cast<void(*)()>(data_obj->data);
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

        const data_object_t* data_obj = get_data_object(pub_channels[channel].object_ids[element]);
        if (data_obj == NULL || !(data_obj->access & TS_READ_ALL)) {
            continue;
        }

        len += json_serialize_name_value(&msg_buf[len], size - len, data_obj);

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
            return status_message_json(TS_STATUS_UNKNOWN_DATA_OBJ);
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