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

int ts_json_serialize_value(struct ts_context *ts, char *buf, size_t size,
                            const struct ts_data_object *object)
{
    int pos = 0;
    struct ts_array_info *array_info;
    float value;

    switch (object->type) {
#if TS_64BIT_TYPES_SUPPORT
    case TS_T_UINT64:
        pos = snprintf(buf, size, "%" PRIu64 ",", *((uint64_t *)object->data));
        break;
    case TS_T_INT64:
        pos = snprintf(buf, size, "%" PRIi64 ",", *((int64_t *)object->data));
        break;
#endif
    case TS_T_UINT32:
        pos = snprintf(buf, size, "%" PRIu32 ",", *((uint32_t *)object->data));
        break;
    case TS_T_INT32:
        pos = snprintf(buf, size, "%" PRIi32 ",", *((int32_t *)object->data));
        break;
    case TS_T_UINT16:
        pos = snprintf(buf, size, "%" PRIu16 ",", *((uint16_t *)object->data));
        break;
    case TS_T_INT16:
        pos = snprintf(buf, size, "%" PRIi16 ",", *((int16_t *)object->data));
        break;
    case TS_T_FLOAT32:
        value = *((float *)object->data);
        if (isnan(value) || isinf(value)) {
            /* JSON spec does not support NaN and Inf, so we need to use null instead */
            return snprintf(buf, size, "null,");
        }
        else {
            pos = snprintf(buf, size, "%.*f,", object->detail, value);
        }
        break;
#if TS_DECFRAC_TYPE_SUPPORT
    case TS_T_DECFRAC:
        pos = snprintf(buf, size, "%" PRIi32 "e%" PRIi16 ",",
            *((uint32_t *)object->data), object->detail);
        break;
#endif
    case TS_T_BOOL:
        pos = snprintf(buf, size, "%s,",
                (*((bool *)object->data) == true ? "true" : "false"));
        break;
    case TS_T_EXEC:
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
        break;
    case TS_T_STRING:
        pos = snprintf(buf, size, "\"%s\",", (char *)object->data);
        break;
    case TS_T_SUBSET:
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
        break;
    case TS_T_ARRAY:
        array_info = (struct ts_array_info *)object->data;
        if (!array_info) {
            return 0;
        }
        pos += snprintf(buf + pos, size - pos, "[");
        for (int i = 0; i < array_info->num_elements; i++) {
            switch (array_info->type) {
            case TS_T_UINT64:
                pos += snprintf(buf + pos, size - pos, "%" PRIu64 ",",
                        ((uint64_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT64:
                pos += snprintf(buf + pos, size - pos, "%" PRIi64 ",",
                        ((int64_t *)array_info->ptr)[i]);
                break;
            case TS_T_UINT32:
                pos += snprintf(buf + pos, size - pos, "%" PRIu32 ",",
                        ((uint32_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT32:
                pos += snprintf(buf + pos, size - pos, "%" PRIi32 ",",
                        ((int32_t *)array_info->ptr)[i]);
                break;
            case TS_T_UINT16:
                pos += snprintf(buf + pos, size - pos, "%" PRIu16 ",",
                        ((uint16_t *)array_info->ptr)[i]);
                break;
            case TS_T_INT16:
                pos += snprintf(buf + pos, size - pos, "%" PRIi16 ",",
                        ((int16_t *)array_info->ptr)[i]);
                break;
            case TS_T_FLOAT32:
                pos += snprintf(buf + pos, size - pos, "%.*f,", object->detail,
                        ((float *)array_info->ptr)[i]);
                break;
            default:
                break;
            }
        }
        if (array_info->num_elements > 0) {
            pos--; // remove trailing comma
        }
        pos += snprintf(buf + pos, size - pos, "],");
        break;
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
                    LOG_DBG("%*s%s", 4 * (level + 1), "", LOG_ALLOC_STR((char *)buf));
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

    const struct ts_data_object *endpoint =
        ts_get_object_by_path(ts, (char *)ts->req + 1, path_len);
    if (!endpoint) {
        if (ts->req[0] == '?' && ts->req[1] == '/' && path_len == 1) {
            return ts_txt_get(ts, NULL, TS_RET_NAMES);
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
                if (endpoint && (endpoint->type == TS_T_GROUP || endpoint->type == TS_T_EXEC)) {
                    return ts_txt_get(ts, endpoint, TS_RET_NAMES);
                }
                else {
                    // device discovery is only allowed for internal objects
                    return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
                }
            }
            else {
                return ts_txt_get(ts, endpoint, TS_RET_NAMES | TS_RET_VALUES);
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

int ts_txt_fetch(struct ts_context *ts, const struct ts_data_object *parent)
{
    int pos = 0;
    int tok = 0;       // current token

    ts_object_id_t parent_id = (parent == NULL) ? 0 : parent->id;

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
            ts->tokens[tok].end - ts->tokens[tok].start, parent_id);

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
        case TS_T_UINT64:
            *((uint64_t*)object->data) = strtoull(buf, NULL, 0);
            break;
        case TS_T_INT64:
            *((int64_t*)object->data) = strtoll(buf, NULL, 0);
            break;
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

int ts_txt_patch(struct ts_context *ts, const struct ts_data_object *parent)
{
    int tok = 0;       // current token
    bool updated = false;

    // buffer for data object value (largest negative 64bit integer has 20 digits)
    char value_buf[21];
    size_t value_len;   // length of value in buffer

    ts_object_id_t parent_id = (parent == NULL) ? 0 : parent->id;

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
            ts->tokens[tok].end - ts->tokens[tok].start, parent_id);

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
                ts->tokens[tok].end - ts->tokens[tok].start, parent_id);

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

int ts_txt_get(struct ts_context *ts, const struct ts_data_object *parent, uint32_t ret_type)
{
    bool include_values = (ret_type & TS_RET_VALUES);

    // initialize response with success message
    size_t len = ts_txt_response(ts, TS_STATUS_CONTENT);

    ts_object_id_t parent_id = (parent == NULL) ? 0 : parent->id;

    if (parent != NULL && parent->type != TS_T_GROUP &&
        parent->type != TS_T_EXEC)
    {
        // get value of data object
        ts->resp[len++] = ' ';
        len += ts_json_serialize_value(ts, (char *)&ts->resp[len], ts->resp_size - len, parent);
        ts->resp[--len] = '\0';     // remove trailing comma again
        return len;
    }

    if (parent != NULL && parent->type == TS_T_EXEC && include_values) {
        // bad request, as we can't read exec object's values
        return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
    }

    len += sprintf((char *)&ts->resp[len], include_values ? " {" : " [");
    int objects_found = 0;
    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if ((ts->data_objects[i].access & TS_READ_MASK) &&
            (ts->data_objects[i].parent == parent_id))
        {
            if (include_values) {
                if (ts->data_objects[i].type == TS_T_GROUP) {
                    // bad request, as we can't read internal path object's values
                    return ts_txt_response(ts, TS_STATUS_BAD_REQUEST);
                }
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

/* currently only supporting nesting of depth 1 */
int ts_txt_export(struct ts_context *ts, char *buf, size_t buf_size, uint16_t subsets)
{
    unsigned int len = 1;
    buf[0] = '{';
    uint16_t prev_parent = 0;
    unsigned int depth = 0;

    for (unsigned int i = 0; i < ts->num_objects; i++) {
        if (ts->data_objects[i].subsets & subsets) {
            const uint16_t parent_id = ts->data_objects[i].parent;
            if (prev_parent != parent_id) {
                if (prev_parent != 0) {
                    // close object of previous parent
                    buf[len-1] = '}';    // overwrite comma
                    buf[len++] = ',';
                }
                struct ts_data_object *parent = ts_get_object_by_id(ts, parent_id);
                len += snprintf(&buf[len], buf_size - len, "\"%s\":{", parent->name);
                prev_parent = parent_id;
                depth = 1;
            }
            len += ts_json_serialize_name_value(ts, &buf[len], buf_size - len,
                &ts->data_objects[i]);
        }
        if (len >= buf_size - 1 - depth) {
            return 0;
        }
    }

    buf[len-1] = '}';    // overwrite comma

    if (depth == 1) {
        buf[len++] = '}';
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

    return len;
}

#endif /* TS_NESTED_JSON */

int ts_txt_statement(struct ts_context *ts, char *buf, size_t buf_size,
                     struct ts_data_object *object)
{
    unsigned int len;

    if (!object || object->parent != 0) {
        // currently only supporting top level objects
        return 0;
    }

    if (object->type == TS_T_SUBSET) {
        len = snprintf(buf, buf_size, "#%s ", object->name);
        len += ts_txt_export(ts, &buf[len], buf_size - len, object->detail);
    }
    else if (object->type == TS_T_GROUP) {
        len = snprintf(buf, buf_size, "#%s {", object->name);
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
