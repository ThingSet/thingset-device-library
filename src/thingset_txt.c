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

int ts_txt_response(struct ts_context *ts, int code)
{
    int pos = 0;

#if TS_VERBOSE_STATUS_MESSAGES
    const char *msg;
    switch (code) {
        // success
        case TS_STATUS_CREATED:
            msg = "Created";
            break;
        case TS_STATUS_DELETED:
            msg = "Deleted";
            break;
        case TS_STATUS_VALID:
            msg = "Valid";
            break;
        case TS_STATUS_CHANGED:
            msg = "Changed";
            break;
        case TS_STATUS_CONTENT:
            msg = "Content";
            break;
        // client errors
        case TS_STATUS_BAD_REQUEST:
            msg = "Bad Request";
            break;
        case TS_STATUS_UNAUTHORIZED:
            msg = "Unauthorized";
            break;
        case TS_STATUS_FORBIDDEN:
            msg = "Forbidden";
            break;
        case TS_STATUS_NOT_FOUND:
            msg = "Not Found";
            break;
        case TS_STATUS_METHOD_NOT_ALLOWED:
            msg = "Method Not Allowed";
            break;
        case TS_STATUS_REQUEST_INCOMPLETE:
            msg = "Request Entity Incomplete";
            break;
        case TS_STATUS_CONFLICT:
            msg = "Conflict";
            break;
        case TS_STATUS_REQUEST_TOO_LARGE:
            msg = "Request Entity Too Large";
            break;
        case TS_STATUS_UNSUPPORTED_FORMAT:
            msg = "Unsupported Content-Format";
            break;
        // server errors
        case TS_STATUS_INTERNAL_SERVER_ERR:
            msg = "Internal Server Error";
            break;
        case TS_STATUS_NOT_IMPLEMENTED:
            msg = "Not Implemented";
            break;
        // ThingSet specific errors
        case TS_STATUS_RESPONSE_TOO_LARGE:
            msg = "Response too large";
            break;
        default:
            msg = "Error";
            break;
    };
    pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X %s.", code, msg);
#else
    pos = snprintf((char *)ts->resp, ts->resp_size, ":%.2X.", code);
#endif
    if (pos < ts->resp_size)
        return pos;
    else
        return 0;
}

static int json_serialize_simple_value(char *buf, size_t size, void *data, int type, int detail)
{
    switch (type) {
#if TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        return snprintf(buf, size, "%" PRIu64 ",", *((uint64_t *)data));
    case TS_T_INT64:
        return snprintf(buf, size, "%" PRIi64 ",", *((int64_t *)data));
#endif
    case TS_T_UINT32:
        return snprintf(buf, size, "%" PRIu32 ",", *((uint32_t *)data));
    case TS_T_INT32:
        return snprintf(buf, size, "%" PRIi32 ",", *((int32_t *)data));
    case TS_T_UINT16:
        return snprintf(buf, size, "%" PRIu16 ",", *((uint16_t *)data));
    case TS_T_INT16:
        return snprintf(buf, size, "%" PRIi16 ",", *((int16_t *)data));
    case TS_T_UINT8:
        return snprintf(buf, size, "%" PRIu8 ",", *((uint8_t *)data));
    case TS_T_INT8:
        return snprintf(buf, size, "%" PRIi8 ",", *((int8_t *)data));
    case TS_T_FLOAT32: {
        float value = *((float *)data);
        if (isnan(value) || isinf(value)) {
            /* JSON spec does not support NaN and Inf, so we need to use null instead */
            return snprintf(buf, size, "null,");
        }
        else {
            return snprintf(buf, size, "%.*f,", detail, value);
        }
    }
#if TS_DECFRAC_TYPE_SUPPORT
    case TS_T_DECFRAC:
        return snprintf(buf, size, "%" PRIi32 "e%" PRIi16 ",", *((uint32_t *)data), detail);
#endif
    case TS_T_BOOL:
        return snprintf(buf, size, "%s,", (*((bool *)data) == true ? "true" : "false"));
    case TS_T_STRING:
        return snprintf(buf, size, "\"%s\",", (char *)data);
    }
    return 0;
}

int ts_json_serialize_value(struct ts_context *ts, char *buf, size_t size,
                            const struct ts_data_object *object)
{
    int pos = json_serialize_simple_value(buf, size, object->data, object->type, object->detail);

    if (pos == 0) {
        // not a simple value
        if (object->type == TS_T_EXEC) {
            pos = snprintf(buf, size, "[");
            for (unsigned int i = 0; i < ts->num_objects; i++) {
                if (ts->data_objects[i].parent == object->id) {
                    pos += snprintf(buf + pos, size - pos, "\"%s\",", ts->data_objects[i].name);
                }
            }
            if (pos > 1) {
                pos--; // remove trailing comma
                pos += snprintf(buf + pos, size - pos, "],");
            }
            else {
                pos = snprintf(buf, size, "null,");
            }
        }
        else if (object->type == TS_T_SUBSET) {
            pos = snprintf(buf, size, "[");
            for (unsigned int i = 0; i < ts->num_objects; i++) {
                if (ts->data_objects[i].subsets & (uint16_t)object->detail) {
#if TS_NESTED_JSON
                    if (ts->data_objects[i].parent == 0) {
                        pos += snprintf(buf + pos, size - pos, "\"%s\",", ts->data_objects[i].name);
                    }
                    else {
                        struct ts_data_object *parent_obj =
                            ts_get_object_by_id(ts, ts->data_objects[i].parent);
                        if (parent_obj != NULL) {
                            pos += snprintf(buf + pos, size - pos, "\"%s/%s\",",
                                            parent_obj->name, ts->data_objects[i].name);
                        }
                    }
#else
                    pos += snprintf(buf + pos, size - pos, "\"%s\",", ts->data_objects[i].name);
#endif
                }
            }
            if (pos > 1) {
                pos--; // remove trailing comma
            }
            pos += snprintf(buf + pos, size - pos, "],");
        }
        else if (object->type == TS_T_ARRAY && object->data != NULL) {
            struct ts_array *array = (struct ts_array *)object->data;
            pos += snprintf(buf + pos, size - pos, "[");
            for (int i = 0; i < array->num_elements; i++) {
                void *data = (uint8_t *)array->elements + i * array->type_size;
                pos += json_serialize_simple_value(buf + pos, size - pos,
                    data, array->type, object->detail);
            }
            if (array->num_elements > 0) {
                pos--; // remove trailing comma
            }
            pos += snprintf(buf + pos, size - pos, "],");
        }
        else if (object->type == TS_T_GROUP || object->type == TS_T_RECORDS) {
            pos = snprintf(buf, size, "null,");
        }
    }

    if (pos < size) {
        return pos;
    }
    else {
        return 0;
    }
}

int ts_json_serialize_name_value(struct ts_context *ts, char *buf, size_t size,
                                 const struct ts_data_object *object)
{
    size_t len_name = snprintf(buf, size, "\"%s\":", object->name);
    if (len_name < 0) {
        return 0;
    }

    int len_value = ts_json_serialize_value(ts, &buf[len_name], size - len_name, object);
    if (len_value < 0) {
        return 0;
    }

    return len_name + len_value;
}

void ts_dump_json(struct ts_context *ts, ts_object_id_t obj_id, int level)
{
    uint8_t buf[100];
    bool first = true;
    if (obj_id == 0) {
        printf("{");
    }
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].parent == obj_id && ts->data_objects[i].type != TS_T_BYTES) {
            if (!first) {
                printf(",\n");
            }
            else {
                printf("\n");
                first = false;
            }
            if (ts->data_objects[i].type == TS_T_GROUP) {
                LOG_DBG("%*s\"%s\": {", 4 * (level + 1), "", ts->data_objects[i].name);
                ts_dump_json(ts, ts->data_objects[i].id, level + 1);
                LOG_DBG("\n%*s}", 4 * (level + 1), "");
            }
            else {
                int pos = ts_json_serialize_name_value(ts, (char *)buf, sizeof(buf),
                    &ts->data_objects[i]);
                if (pos > 0) {
                    buf[pos-1] = '\0';  // remove trailing comma
                    LOG_DBG("%*s%s", 4 * (level + 1), "", (char *)buf);
                }
            }
        }
    }
    if (obj_id == 0) {
        LOG_DBG("\n}\n");
    }
}

int ts_txt_process(struct ts_context *ts)
{
    int path_len = ts->req_len - 1;
    char *path_end = strchr((char *)ts->req + 1, ' ');
    if (path_end) {
        path_len = (uint8_t *)path_end - ts->req - 1;
    }

    int record_index = RECORD_INDEX_NONE;
    const struct ts_data_object *endpoint =
        ts_get_endpoint_by_path(ts, (char *)ts->req + 1, path_len, &record_index);
    if (!endpoint) {
        if (ts->req[0] == '?' && ts->req[1] == '/' && path_len == 1) {
            return ts_txt_get(ts, NULL, TS_RET_NAMES, record_index);
        }
        else if (path_len > 0) {
            return ts_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
    }

    jsmn_parser parser;
    jsmn_init(&parser);

    ts->json_str = (char *)ts->req + 1 + path_len;
    ts->tok_count = jsmn_parse(&parser, ts->json_str, ts->req_len - path_len - 1,
        ts->tokens, sizeof(ts->tokens));

    if (ts->tok_count == JSMN_ERROR_NOMEM) {
        return ts_txt_response(ts, TS_STATUS_REQUEST_TOO_LARGE);
    }
    else if (ts->tok_count < 0) {
        // other parsing error
        return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
    }
    else if (ts->tok_count == 0) {
        if (ts->req[0] == '?') {
            // no payload data
            if ((char)ts->req[path_len] == '/') {
                if (endpoint && (endpoint->type == TS_T_GROUP || endpoint->type == TS_T_EXEC ||
                    endpoint->type == TS_T_RECORDS))
                {
                    return ts_txt_get(ts, endpoint, TS_RET_NAMES, record_index);
                }
                else {
                    // device discovery is only allowed for internal objects
                    return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
                }
            }
            else {
                return ts_txt_get(ts, endpoint, TS_RET_NAMES | TS_RET_VALUES, record_index);
            }
        }
        else if (ts->req[0] == '!') {
            return ts_txt_exec(ts, endpoint);
        }
    }
    else {
        if (ts->req[0] == '?') {
            return ts_txt_fetch(ts, endpoint);
        }
        else if (ts->req[0] == '=') {
            int len = ts_txt_patch(ts, endpoint);

            // check if endpoint has a callback assigned
            if (endpoint && endpoint->data != NULL && strncmp((char *)ts->resp, ":84", 3) == 0) {
                // create function pointer and call function
                void (*fun)(void) = (void(*)(void))endpoint->data;
                fun();
            }
            return len;
        }
        else if (ts->req[0] == '!' && endpoint && endpoint->type == TS_T_EXEC) {
            return ts_txt_exec(ts, endpoint);
        }
        else if (ts->req[0] == '+') {
            return ts_txt_create(ts, endpoint);
        }
        else if (ts->req[0] == '-') {
            return ts_txt_delete(ts, endpoint);
        }
    }
    return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
}

int ts_txt_fetch(struct ts_context *ts, const struct ts_data_object *endpoint)
{
    int pos = 0;
    int tok = 0;       // current token

    ts_object_id_t endpoint_id = (endpoint == NULL) ? 0 : endpoint->id;

    // initialize response with success message
    pos += ts_txt_response(ts, TS_STATUS_CONTENT);

    if (ts->tokens[0].type == JSMN_ARRAY) {
        pos += snprintf((char *)&ts->resp[pos], ts->resp_size - pos, " [");
        tok++;
    } else {
        pos += snprintf((char *)&ts->resp[pos], ts->resp_size - pos, " ");
    }

    while (tok < ts->tok_count) {

        if (ts->tokens[tok].type != JSMN_STRING) {
            return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
        }

        const struct ts_data_object *object = ts_get_object_by_name(ts,
            ts->json_str + ts->tokens[tok].start,
            ts->tokens[tok].end - ts->tokens[tok].start, endpoint_id);

        if (object == NULL) {
            return ts_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
        else if (object->type == TS_T_GROUP) {
            // bad request, as we can't read internal path object's values
            return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
        }

        if ((object->access & TS_READ_MASK & ts->_auth_flags) == 0) {
            if (object->access & TS_READ_MASK) {
                return ts_txt_response(ts, TS_STATUS_UNAUTHORIZED);
            }
            else {
                return ts_txt_response(ts, TS_STATUS_FORBIDDEN);
            }
        }

        pos += ts_json_serialize_value(ts, (char *)&ts->resp[pos], ts->resp_size - pos, object);

        if (pos >= ts->resp_size - 2) {
            return ts_txt_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
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

int ts_json_deserialize_value(struct ts_context *ts, char *buf, size_t len, jsmntype_t type,
                              const struct ts_data_object *object)
{
#if TS_DECFRAC_TYPE_SUPPORT
    float tmp;
#endif

    if (type != JSMN_PRIMITIVE && type != JSMN_STRING) {
        return 0;
    }

    errno = 0;
    switch (object->type) {
        case TS_T_FLOAT32:
            *((float*)object->data) = strtod(buf, NULL);
            break;
#if TS_DECFRAC_TYPE_SUPPORT
        case TS_T_DECFRAC:
            tmp = strtod(buf, NULL);
            // positive exponent
            for (int16_t i = 0; i < object->detail; i++) {
                tmp /= 10.0F;
            }
            // negative exponent
            for (int16_t i = 0; i > object->detail; i--) {
                tmp *= 10.0F;
            }
            *((int32_t*)object->data) = (int32_t)tmp;
            break;
#endif
#if TS_64BIT_TYPES_SUPPORT
        case TS_T_UINT64:
            *((uint64_t*)object->data) = strtoull(buf, NULL, 0);
            break;
        case TS_T_INT64:
            *((int64_t*)object->data) = strtoll(buf, NULL, 0);
            break;
#endif
        case TS_T_UINT32:
            *((uint32_t*)object->data) = strtoul(buf, NULL, 0);
            break;
        case TS_T_INT32:
            *((int32_t*)object->data) = strtol(buf, NULL, 0);
            break;
        case TS_T_UINT16:
            *((uint16_t*)object->data) = strtoul(buf, NULL, 0);
            break;
        case TS_T_INT16:
            *((uint16_t*)object->data) = strtol(buf, NULL, 0);
            break;
        case TS_T_UINT8:
            *((uint8_t*)object->data) = strtoul(buf, NULL, 0);
            break;
        case TS_T_INT8:
            *((uint8_t*)object->data) = strtol(buf, NULL, 0);
            break;
        case TS_T_BOOL:
            if (buf[0] == 't' || buf[0] == '1') {
                *((bool*)object->data) = true;
            }
            else if (buf[0] == 'f' || buf[0] == '0') {
                *((bool*)object->data) = false;
            }
            else {
                return 0;       // error
            }
            break;
        case TS_T_STRING:
            if (type != JSMN_STRING || (unsigned int)object->detail <= len) {
                return 0;
            }
            else if (object->id != 0) {     // dummy object has id = 0
                strncpy((char*)object->data, buf, len);
                ((char*)object->data)[len] = '\0';
            }
            break;
    }

    if (errno == ERANGE) {
        return 0;
    }

    return 1;   // value always contained in one token (arrays not yet supported)
}

int ts_txt_patch(struct ts_context *ts, const struct ts_data_object *endpoint)
{
    int tok = 0;       // current token
    bool updated = false;

    // buffer for data object value (largest negative 64bit integer has 20 digits)
    char value_buf[21];
    size_t value_len;   // length of value in buffer

    ts_object_id_t endpoint_id = (endpoint == NULL) ? 0 : endpoint->id;

    if (ts->tok_count < 2) {
        if (ts->tok_count == JSMN_ERROR_NOMEM) {
            return ts_txt_response(ts, TS_STATUS_REQUEST_TOO_LARGE);
        } else {
            return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
        }
    }

    if (ts->tokens[0].type == JSMN_OBJECT) {    // object = map
        tok++;
    }

    // loop through all elements to check if request is valid
    while (tok + 1 < ts->tok_count) {

        if (ts->tokens[tok].type != JSMN_STRING ||
            (ts->tokens[tok+1].type != JSMN_PRIMITIVE && ts->tokens[tok+1].type != JSMN_STRING)) {
            return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
        }

        const struct ts_data_object* object = ts_get_object_by_name(ts,
            ts->json_str + ts->tokens[tok].start,
            ts->tokens[tok].end - ts->tokens[tok].start, endpoint_id);

        if (object == NULL) {
            return ts_txt_response(ts, TS_STATUS_NOT_FOUND);
        }

        if ((object->access & TS_WRITE_MASK & ts->_auth_flags) == 0) {
            if (object->access & TS_WRITE_MASK) {
                return ts_txt_response(ts, TS_STATUS_UNAUTHORIZED);
            }
            else {
                return ts_txt_response(ts, TS_STATUS_FORBIDDEN);
            }
        }

        tok++;

        // extract the value and check buffer lengths
        value_len = ts->tokens[tok].end - ts->tokens[tok].start;
        if (object->type == TS_T_STRING) {
            if (value_len < (size_t)object->detail) {
                // provided string fits into data object buffer
                tok += 1;
                continue;
            }
            else {
                return ts_txt_response(ts, TS_STATUS_REQUEST_TOO_LARGE);
            }
        }
        else if (value_len >= sizeof(value_buf)) {
            return ts_txt_response(ts, TS_STATUS_UNSUPPORTED_FORMAT);
        }
        else {
            strncpy(value_buf, &ts->json_str[ts->tokens[tok].start], value_len);
            value_buf[value_len] = '\0';
        }

        // create dummy object to test formats
        uint8_t dummy_data[8];          // enough to fit also 64-bit values
        struct ts_data_object dummy_object = {0, 0, "Dummy", (void *)dummy_data, object->type,
            object->detail};

        int res = ts_json_deserialize_value(ts, value_buf, value_len, ts->tokens[tok].type,
            &dummy_object);
        if (res == 0) {
            return ts_txt_response(ts, TS_STATUS_UNSUPPORTED_FORMAT);
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

        const struct ts_data_object *object =
            ts_get_object_by_name(ts, ts->json_str + ts->tokens[tok].start,
                ts->tokens[tok].end - ts->tokens[tok].start, endpoint_id);

        tok++;

        // extract the value again (max. size was checked before)
        value_len = ts->tokens[tok].end - ts->tokens[tok].start;
        if (value_len < sizeof(value_buf)) {
            strncpy(value_buf, &ts->json_str[ts->tokens[tok].start], value_len);
            value_buf[value_len] = '\0';
        }

        tok += ts_json_deserialize_value(ts, &ts->json_str[ts->tokens[tok].start], value_len,
            ts->tokens[tok].type, object);

        if (ts->_update_subsets & object->subsets) {
            updated = true;
        }
    }

    if (updated && ts->update_cb != NULL) {
        ts->update_cb();
    }

    return ts_txt_response(ts, TS_STATUS_CHANGED);
}

int ts_txt_get(struct ts_context *ts, const struct ts_data_object *endpoint, uint32_t ret_type,
               int record_index)
{
    bool include_values = (ret_type & TS_RET_VALUES);

    // initialize response with success message
    size_t len = ts_txt_response(ts, TS_STATUS_CONTENT);

    ts_object_id_t endpoint_id = 0;

    if (endpoint != NULL) {
        switch (endpoint->type) {
            case TS_T_EXEC:
                if (include_values) {
                    // bad request, as we can't read exec object's values
                    return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
                }
                break;
            case TS_T_GROUP:
                break;
            case TS_T_RECORDS:
                if (ret_type == TS_RET_NAMES) {
                    struct ts_records *records = (struct ts_records *)endpoint->data;
                    len += snprintf((char *)&ts->resp[len], ts->resp_size - len, " %d",
                        records->num_records);
                    return len;
                }
                break;
            default:
                // get value of data object
                ts->resp[len++] = ' ';
                len += ts_json_serialize_value(ts, (char *)&ts->resp[len], ts->resp_size - len,
                    endpoint);
                ts->resp[--len] = '\0'; // remove trailing comma again
                return len;
        }
        endpoint_id = endpoint->id;
    }

    len += sprintf((char *)&ts->resp[len], include_values ? " {" : " [");
    int objects_found = 0;
    if (endpoint && endpoint->type == TS_T_RECORDS) {
        struct ts_records *records = (struct ts_records *)endpoint->data;

        if (record_index == RECORD_INDEX_NONE) {
            len--;
            len += snprintf((char *)ts->resp + len, ts->resp_size - len, "null");
            return len;
        }

        /* record item definitions are expected to start behind endpoint data object */
        const struct ts_data_object *item = endpoint + 1;
        while (item < &ts->data_objects[ts->num_objects] && item->parent == endpoint->id) {
            size_t len_name = snprintf((char *)ts->resp + len, ts->resp_size - len, "\"%s\":",
                item->name);
            if (len_name < 0) {
                return 0;
            }

            void *data = (uint8_t *)records->data + record_index * records->record_size +
                (size_t)item->data;
            int len_value = json_serialize_simple_value((char *)ts->resp + len + len_name,
                ts->resp_size - len - len_name, data, item->type, item->detail);
            if (len_value < 0) {
                return 0;
            }

            len += len_name + len_value;
            objects_found++;
            item++;
        }
    }
    else {
        for (unsigned int i = 0; i < ts->num_objects; i++) {
            if ((ts->data_objects[i].access & TS_READ_MASK) &&
                (ts->data_objects[i].parent == endpoint_id))
            {
                if (include_values) {
                    int ret = ts_json_serialize_name_value(ts, (char *)&ts->resp[len],
                        ts->resp_size - len, &ts->data_objects[i]);
                    if (ret > 0) {
                        len += ret;
                    }
                    else {
                        return ts_txt_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
                    }
                }
                else {
                    len += snprintf((char *)&ts->resp[len],
                        ts->resp_size - len,
                        "\"%s\",", ts->data_objects[i].name);
                }
                objects_found++;

                if (len >= ts->resp_size - 1) {
                    return ts_txt_response(ts, TS_STATUS_RESPONSE_TOO_LARGE);
                }
            }
        }
    }

    // remove trailing comma and add closing bracket
    if (objects_found == 0) {
        len++;
    }
    ts->resp[len-1] = include_values ? '}' : ']';
    ts->resp[len] = '\0';

    return len;
}

int ts_txt_create(struct ts_context *ts, const struct ts_data_object *object)
{
    if (ts->tok_count > 1) {
        // only single JSON primitive supported at the moment
        return ts_txt_response(ts, TS_STATUS_NOT_IMPLEMENTED);
    }

    if (object->type == TS_T_ARRAY) {
        // Remark: See commit history with implementation for pub/sub ID arrays as inspiration
        return ts_txt_response(ts, TS_STATUS_NOT_IMPLEMENTED);
    }
    else if (object->type == TS_T_SUBSET) {
        if (ts->tokens[0].type == JSMN_STRING) {
#if TS_NESTED_JSON
            struct ts_data_object *add_object = ts_get_object_by_path(ts, ts->json_str +
                ts->tokens[0].start, ts->tokens[0].end - ts->tokens[0].start);
#else
            struct ts_data_object *add_object = ts_get_object_by_name(ts, ts->json_str +
                ts->tokens[0].start, ts->tokens[0].end - ts->tokens[0].start, -1);
#endif
            if (add_object != NULL) {
                add_object->subsets |= (uint16_t)object->detail;
                return ts_txt_response(ts, TS_STATUS_CREATED);
            }
            return ts_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
    }
    return ts_txt_response(ts, TS_STATUS_METHOD_NOT_ALLOWED);
}

int ts_txt_delete(struct ts_context *ts, const struct ts_data_object *object)
{
    if (ts->tok_count > 1) {
        // only single JSON primitive supported at the moment
        return ts_txt_response(ts, TS_STATUS_NOT_IMPLEMENTED);
    }

    if (object->type == TS_T_ARRAY) {
        // Remark: See commit history with implementation for pub/sub ID arrays as inspiration
        return ts_txt_response(ts, TS_STATUS_NOT_IMPLEMENTED);
    }
    else if (object->type == TS_T_SUBSET) {
        if (ts->tokens[0].type == JSMN_STRING) {
#if TS_NESTED_JSON
            struct ts_data_object *del_object = ts_get_object_by_path(ts, ts->json_str +
                ts->tokens[0].start, ts->tokens[0].end - ts->tokens[0].start);
#else
            struct ts_data_object *del_object = ts_get_object_by_name(ts, ts->json_str +
                ts->tokens[0].start, ts->tokens[0].end - ts->tokens[0].start, -1);
#endif
            if (del_object != NULL) {
                del_object->subsets &= ~((uint16_t)object->detail);
                return ts_txt_response(ts, TS_STATUS_DELETED);
            }
            return ts_txt_response(ts, TS_STATUS_NOT_FOUND);
        }
    }
    return ts_txt_response(ts, TS_STATUS_METHOD_NOT_ALLOWED);
}

int ts_txt_exec(struct ts_context *ts, const struct ts_data_object *object)
{
    int tok = 0;            // current token
    int objects_found = 0;    // number of child objects found

    if (ts->tok_count > 0 && ts->tokens[tok].type == JSMN_ARRAY) {
        tok++;      // go to first element of array
    }

    if ((object->access & TS_WRITE_MASK) && (object->type == TS_T_EXEC)) {
        // object is generally executable, but are we authorized?
        if ((object->access & TS_WRITE_MASK & ts->_auth_flags) == 0) {
            return ts_txt_response(ts, TS_STATUS_UNAUTHORIZED);
        }
    }
    else {
        return ts_txt_response(ts, TS_STATUS_FORBIDDEN);
    }

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].parent == object->id) {
            if (tok >= ts->tok_count) {
                // more child objects found than parameters were passed
                return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
            }
            int res = ts_json_deserialize_value(ts, ts->json_str + ts->tokens[tok].start,
                ts->tokens[tok].end - ts->tokens[tok].start, ts->tokens[tok].type,
                &ts->data_objects[i]);
            if (res == 0) {
                // deserializing the value was not successful
                return ts_txt_response(ts, TS_STATUS_UNSUPPORTED_FORMAT);
            }
            tok += res;
            objects_found++;
        }
    }

    if (ts->tok_count > tok) {
        // more parameters passed than child objects found
        return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
    }

    // if we got here, finally create function pointer and call function
    void (*fun)(void) = (void(*)(void))object->data;
    fun();

    return ts_txt_response(ts, TS_STATUS_VALID);
}

#if TS_NESTED_JSON

/* currently only supporting nesting of depth 2 (parent and grandparent != 0) */
int ts_txt_export(struct ts_context *ts, char *buf, size_t buf_size, uint16_t subsets)
{
    struct ts_data_object *ancestors[2];
    int depth = 0;
    int len = 1;
    buf[0] = '{';

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].subsets & subsets) {
            const uint16_t parent_id = ts->data_objects[i].parent;
            if (depth > 0 && parent_id != ancestors[depth - 1]->id) {
                // close object of previous parent
                buf[len-1] = '}';    // overwrite comma
                buf[len++] = ',';
                depth--;
            }

            if (depth == 0 && parent_id != 0) {
                struct ts_data_object *parent = ts_get_object_by_id(ts, parent_id);
                if (parent != NULL) {
                    if (parent->parent != 0) {
                        struct ts_data_object *grandparent = ts_get_object_by_id(ts,
                            parent->parent);
                        if (grandparent != NULL) {
                            len += snprintf(&buf[len], buf_size - len, "\"%s\":{",
                                grandparent->name);
                            ancestors[depth++] = grandparent;
                        }
                    }
                    len += snprintf(&buf[len], buf_size - len, "\"%s\":{", parent->name);
                    ancestors[depth++] = parent;
                }
            }
            else if (depth > 0 && parent_id != ancestors[depth - 1]->id) {
                struct ts_data_object *parent = ts_get_object_by_id(ts, parent_id);
                if (parent != NULL) {
                    len += snprintf(&buf[len], buf_size - len, "\"%s\":{", parent->name);
                    ancestors[depth++] = parent;
                }

            }
            len += ts_json_serialize_name_value(ts, &buf[len], buf_size - len,
                &ts->data_objects[i]);
        }
        if (len >= buf_size - 1 - depth) {
            return 0;
        }
    }

    len--; // to overwrite comma

    while (depth >= 0) {
        buf[len++] = '}';
        buf[len] = '\0';
        depth--;
    }

    return len;
}

#else

int ts_txt_export(struct ts_context *ts, char *buf, size_t buf_size, uint16_t subsets)
{
    unsigned int len = 1;
    buf[0] = '{';

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].subsets & subsets) {
            len += ts_json_serialize_name_value(ts, &buf[len], buf_size - len,
                &ts->data_objects[i]);
        }
        if (len >= buf_size - 1) {
            return 0;
        }
    }

    buf[len-1] = '}';    // overwrite comma
    buf[len] = '\0';

    return len;
}

#endif /* TS_NESTED_JSON */

int ts_txt_statement(struct ts_context *ts, char *buf, size_t buf_size,
                     struct ts_data_object *object)
{
    unsigned int len = 0;

    if (!object) {
        return 0;
    }

    buf[len++] = '#';
    len += ts_get_path(ts, &buf[len], buf_size - len, object);

    if (len > 1 && buf_size > len) {
        buf[len++] = ' ';
    }
    else {
        return 0;
    }

    if (object->type == TS_T_SUBSET) {
        len += ts_txt_export(ts, &buf[len], buf_size - len, object->detail);
    }
    else if (object->type == TS_T_GROUP) {
        buf[len++] = '{';
        for (unsigned int i = 0; i < ts->num_objects; i++) {
            if (ts->data_objects[i].parent == object->id) {
                len += ts_json_serialize_name_value(ts, &buf[len], buf_size - len,
                    &ts->data_objects[i]);
            }
            if (len >= buf_size - 1) {
                return 0;
            }
        }
        buf[len-1] = '}';    // overwrite comma
        buf[len] = '\0';
    }
    else {
        return 0;
    }

    return len;
}

int ts_txt_statement_by_path(struct ts_context *ts, char *buf, size_t buf_size, const char *path)
{
    return ts_txt_statement(ts, buf, buf_size, ts_get_object_by_path(ts, path, strlen(path)));
}

int ts_txt_statement_by_id(struct ts_context *ts, char *buf, size_t buf_size, ts_object_id_t id)
{
    return ts_txt_statement(ts, buf, buf_size, ts_get_object_by_id(ts, id));
}
