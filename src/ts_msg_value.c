/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Value primitives
 * ----------------
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ts_msg.h"

/* Interface */

int ts_msg_add_mem(struct thingset_msg* msg, const uint8_t *mem, uint16_t len)
{
    if (len > ts_msg_tailroom(msg)) {
        TS_LOGD("PRIMITIVE: %s faulting - can not add memory: %u, avail: %u",
                __func__, (unsigned int)len, (unsigned int)ts_msg_tailroom(msg));
        return -ENOMEM;
    }
    (void)ts_buf_add_mem((struct ts_buf *)msg, mem, len);
    TS_LOGD("PRIMITIVE: %s advance by %u", __func__, len);
    return 0;
}

int ts_msg_add_u8(struct thingset_msg* msg, uint8_t val)
{
    if (1 > ts_msg_tailroom(msg)) {
        TS_LOGD("PRIMITIVE: %s faulting - can not add byte: %u, avail: %u",
                __func__, (unsigned int)1, (unsigned int)ts_msg_tailroom(msg));
        return -ENOMEM;
    }
    (void)ts_buf_add_u8((struct ts_buf *)msg, val);
    TS_LOGD("PRIMITIVE: %s advance by %u", __func__, 1);
    return 0;
}

int ts_msg_pull_u8(struct thingset_msg* msg, uint8_t *val)
{
    if (ts_msg_len(msg) == 0) {
        return -ENOMEM;
    }
    *val = *ts_msg_data(msg);
    (void)ts_msg_pull(msg, 1);
    return 0;
}

int ts_msg_pull_path(struct thingset_msg* msg, const char **path, uint16_t *len)
{
    uint16_t path_len = 0;
    uint16_t path_max_len = ts_msg_len(msg);
    const char *path_s = (const char *)ts_msg_data(msg);

    /* Skip space */
    uint16_t spaces = 0;
    while (spaces < path_max_len) {
        if (!isspace(path_s[spaces])) {
            break;
        }
        spaces++;
    }
    if (spaces >= path_max_len) {
        /* Spaces at end of message/ end of message -> take it as empty path */
        *path = NULL;
        goto ts_msg_pull_path_end;
    }

    /* Get path */
    path_len = strspn(&path_s[spaces],
                      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._-/");
    if (path_len > (path_max_len - spaces)) {
        path_len = path_max_len - spaces;
    }
    if (path_len == 0) {
        if (spaces > 0) {
            /* There is space - take it as empty path */
            *path = NULL;
            goto ts_msg_pull_path_end;
        }
        return -EINVAL;
    }
    *path = path_s;

ts_msg_pull_path_end:
    *len = path_len;
    (void)ts_msg_pull(msg, spaces + path_len);
    return 0;
}

/* JSON values */

int ts_msg_add_array_json(struct thingset_msg* msg, uint16_t num_elements)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    if (scratchpad->current >= 2) {
        TS_LOGD("JSON encoder: %s without more encoders", __func__);
        return -ENOMEM;
    }

    scratchpad->current++;

    /* Initialise new encoder */
    struct ts_msg_json_encoder *array_encoder = ts_msg_scratchpad_json_enc_current(msg);
    array_encoder->flags.array = 0;
    array_encoder->flags.array_unbounded = 0;
    array_encoder->flags.map = 0;
    array_encoder->flags.map_unbounded = 0;
    array_encoder->remaining = 0;

    int ret = ts_msg_scratchpad_json_enc_update(msg, __func__, "[", 1);
    if (ret != 0) {
        scratchpad->current--;
        return ret;
    }

    if (num_elements == 0) {
        array_encoder->flags.array_unbounded = 1;
    }
    else {
        array_encoder->flags.array = 1;
    }
    /* Next element to come is a value - always for arrays */
    array_encoder->flags.key = 0;
    array_encoder->remaining = num_elements;

    return 0;
}

int ts_msg_add_array_end_json(struct thingset_msg* msg)
{
    if (ts_msg_scratchpad_json_enc_is_top(msg)) {
        /* Can not close on top level encoder */
        TS_LOGD("JSON encoder: %s close while on top level encoder", __func__);
        return -ENOMEM;
    }

    struct ts_msg_json_encoder *array_encoder = ts_msg_scratchpad_json_enc_current(msg);
    TS_ASSERT(!(array_encoder->flags.map && array_encoder->flags.map_unbounded),
              "JSON encoder: %s unbounded and bounded array defined at same time",__func__);
    if (!array_encoder->flags.array && !array_encoder->flags.array_unbounded) {
        TS_LOGD("JSON encoder: %s close on non array encoder", __func__);
        return -EINVAL;
    }

    int ret = 0;
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);

    /* Check value insertions */
    if (array_encoder->remaining > 0) {
        TS_LOGD("JSON encoder: %s close %s%s array on missing elements (%u)", __func__,
                array_encoder->flags.map ? "bounded" : "",
                array_encoder->flags.map_unbounded ? "unbounded" : "",
                (unsigned int)array_encoder->remaining);
        ret = -EINVAL;
        goto ts_msg_add_array_end_json_end;
    }

    if (scratchpad->instance[scratchpad->current].flags.array_unbounded) {
        /* Remove trailing comma */
        if (ts_msg_tail(msg)[-1] == ',') {
            ts_msg_remove(msg, 1);
        }
    }

ts_msg_add_array_end_json_end:

    scratchpad->current--;

    int ret_update = ts_msg_scratchpad_json_enc_update(msg, __func__, "]", 1);
    if (ret == 0) {
        ret = ret_update;
    }

    return ret;
}

int ts_msg_add_bool_json(struct thingset_msg* msg, bool b)
{
    const char *s;
    uint16_t len;
    if (b) {
        s = "true";
        len = sizeof("true") - 1;
    } else {
        s = "false";
        len = sizeof("false") - 1;
    }
    return ts_msg_scratchpad_json_enc_update(msg, __func__, s, len);
}

int ts_msg_add_decfrac_json(struct thingset_msg* msg, int32_t mantissa, int16_t exponent)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "%" PRIi32 "e%" PRIi16,
                            mantissa, exponent);
    return ts_msg_scratchpad_json_enc_update(msg, __func__,
                                             (const uint8_t *)&scratchpad->s[0], len);
}

int ts_msg_add_f32_json(struct thingset_msg* msg, float val, int precision)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t len;
    if (isnan(val) || isinf(val)) {
        /* JSON spec does not support NaN and Inf, so we need to use null instead */
        len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "null");
    }
    else {
        len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "%.*f", precision, val);
    }
    return ts_msg_scratchpad_json_enc_update(msg, __func__,
                                             (const uint8_t *)&scratchpad->s[0], len);
}

int ts_msg_add_i16_json(struct thingset_msg* msg, int16_t val)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "%" PRIi16, val);
    return ts_msg_scratchpad_json_enc_update(msg, __func__,
                                             (const uint8_t *)&scratchpad->s[0], len);
}

int ts_msg_add_i32_json(struct thingset_msg* msg, int32_t val)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "%" PRIi32, val);
    return ts_msg_scratchpad_json_enc_update(msg, __func__,
                                             (const uint8_t *)&scratchpad->s[0], len);
}

int ts_msg_add_i64_json(struct thingset_msg* msg, int64_t val)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "%" PRIi64, val);
    return ts_msg_scratchpad_json_enc_update(msg, __func__,
                                             (const uint8_t *)&scratchpad->s[0], len);
}

int ts_msg_add_map_json(struct thingset_msg* msg, uint16_t num_elements)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    if (scratchpad->current >= 2) {
        TS_LOGD("JSON encoder: %s without more encoders", __func__);
        return -ENOMEM;
    }

    scratchpad->current++;

    /* Initialise new encoder */
    struct ts_msg_json_encoder *map_encoder = ts_msg_scratchpad_json_enc_current(msg);
    map_encoder->flags.array = 0;
    map_encoder->flags.array_unbounded = 0;
    map_encoder->flags.map = 0;
    map_encoder->flags.map_unbounded = 0;
    map_encoder->remaining = 0;

    int ret = ts_msg_scratchpad_json_enc_update(msg, __func__, "{", 1);
    if (ret != 0) {
        scratchpad->current--;
        return ret;
    }

    if (num_elements == 0) {
        map_encoder->flags.map_unbounded = 1;
    }
    else {
        map_encoder->flags.map = 1;
    }
    /* Next element to come is a key */
    map_encoder->flags.key = 1;
    map_encoder->remaining = num_elements;

    return 0;
}

int ts_msg_add_map_end_json(struct thingset_msg* msg)
{
    if (ts_msg_scratchpad_json_enc_is_top(msg)) {
        /* Can not close on top level encoder */
        TS_LOGD("JSON encoder: %s close while on top level encoder", __func__);
        return -ENOMEM;
    }

    struct ts_msg_json_encoder *map_encoder = ts_msg_scratchpad_json_enc_current(msg);
    TS_ASSERT(!(map_encoder->flags.map && map_encoder->flags.map_unbounded),
              "JSON encoder: %s unbounded and bounded map defined at same time",__func__);
    if (!map_encoder->flags.map && !map_encoder->flags.map_unbounded) {
        TS_LOGD("JSON encoder: %s close on non map encoder", __func__);
        return -EINVAL;
    }

    int ret = 0;
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);

    /* Check map key:value insertions */
    if (map_encoder->remaining > 0) {
        TS_LOGD("JSON encoder: %s close %s%s map on missing elements (%u)", __func__,
                map_encoder->flags.map ? "bounded" : "",
                map_encoder->flags.map_unbounded ? "unbounded" : "",
                (unsigned int)map_encoder->remaining);
        ret = -EINVAL;
        goto ts_msg_add_map_end_json_end;
    }
    if (!map_encoder->flags.key) {
        /* Value to come - last element added was a key. */
        TS_LOGD("JSON encoder: %s close %s%s map on key element", __func__,
                map_encoder->flags.map ? "bounded" : "",
                map_encoder->flags.map_unbounded ? "unbounded" : "");
        ret = -EINVAL;
        goto ts_msg_add_map_end_json_end;
    }

    if (scratchpad->instance[scratchpad->current].flags.map_unbounded) {
        /* Remove trailing comma */
        if (ts_msg_tail(msg)[-1] == ',') {
            ts_msg_remove(msg, 1);
        }
    }

ts_msg_add_map_end_json_end:

    scratchpad->current--;

    int ret_update = ts_msg_scratchpad_json_enc_update(msg, __func__, "}", 1);
    if (ret == 0) {
        ret = ret_update;
    }

    return ret;
}

int ts_msg_add_string_json(struct thingset_msg* msg, const char *s)
{
    /* additional 2x" (+2), trailing 0 for snprintf (+1) */
    uint16_t len = strnlen(s, ts_msg_tailroom(msg)) + 3;
    if (len > ts_msg_tailroom(msg)) {
        return -ENOMEM;
    }
    (void)snprintf((char *)ts_msg_tail(msg), len, "\"%s\"", s);
    return ts_msg_scratchpad_json_enc_update(msg, __func__, NULL, len - 1);
}

int ts_msg_add_u16_json(struct thingset_msg* msg, uint16_t val)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "%" PRIu16, val);
    return ts_msg_scratchpad_json_enc_update(msg, __func__,
                                             (const uint8_t *)&scratchpad->s[0], len);
}

int ts_msg_add_u32_json(struct thingset_msg* msg, uint32_t val)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "%" PRIu32, val);
    return ts_msg_scratchpad_json_enc_update(msg, __func__,
                                                (const uint8_t *)&scratchpad->s[0], len);
}


int ts_msg_add_u64_json(struct thingset_msg* msg, uint64_t val)
{
    TS_MSG_JSON_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t len = snprintf(&scratchpad->s[0], sizeof(scratchpad->s) - 1, "%" PRIu64, val);
    return ts_msg_scratchpad_json_enc_update(msg, __func__,
                                             (const uint8_t *)&scratchpad->s[0], len);
}

int ts_msg_pull_type_json(struct thingset_msg* msg, uint16_t *jsmn_type)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    *jsmn_type = type;
    return 0;
}

int ts_msg_pull_array_json(struct thingset_msg* msg, uint16_t *num_elements)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_ARRAY) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }
    *num_elements = size;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_array_end_json(struct thingset_msg* msg)
{
    return 0;
}

int ts_msg_pull_bool_json(struct thingset_msg* msg, bool *b)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    if (*start == 't' || *start == '1') {
        *b = true;
    }
    else if (*start == 'f' || *start == '0') {
        *b = false;
    }
    else {
        TS_LOGD("JSON decoder: %s '%.*s' is not a bool", __func__, len, start);
        return -EINVAL;
    }
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_decfrac_json(struct thingset_msg* msg, int32_t *mantissa, int16_t *exponent)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    int32_t val_mantissa;
    int16_t val_exponent;
    char *endptr = NULL;
    val_mantissa = strtol(start, &endptr, 10);
    if (endptr > (start + len)) {
        TS_LOGD("JSON decoder: %s '%.*s' is not a decfrac", __func__, len, start);
        return -EINVAL;
    }
    if (endptr == (start + len)) {
        val_exponent = 0;
    }
    else if ((*endptr != 'e') && (*endptr != 'E')) {
        TS_LOGD("JSON decoder: %s '%.*s' is not a decfrac", __func__, len, start);
        return -EINVAL;
    }
    else {
        endptr++;
        val_exponent = (int16_t)strtol(endptr, &endptr, 10);
        if (endptr != (start + len)) {
            TS_LOGD("JSON decoder: %s '%.*s' is not a decfrac", __func__, len, start);
            return -EINVAL;
        }
    }

    *mantissa = val_mantissa;
    *exponent = val_exponent;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_f32_json(struct thingset_msg* msg, float *val)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    float val_float;
    if (strcmp(start, "null") == 0) {
        val_float = NAN;
    }
    else {
        char *endptr = NULL;
        val_float = strtod(start, &endptr);
        if (endptr != (start + len)) {
            TS_LOGD("JSON decoder: %s '%.*s' is not a f32", __func__, len, start);
            return -EINVAL;
        }
    }
    *val = val_float;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_i16_json(struct thingset_msg* msg, int16_t *val)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    char *endptr = NULL;
    int16_t val_i16 = (int16_t)strtol(start, &endptr, 10);
    if (endptr != (start + len)) {
        TS_LOGD("JSON decoder: %s '%.*s' is not an i16", __func__, len, start);
        return -EINVAL;
    }
    *val = val_i16;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_i32_json(struct thingset_msg* msg, int32_t *val)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    char *endptr = NULL;
    int32_t val_i32 = strtol(start, &endptr, 10);
    if (endptr != (start + len)) {
        TS_LOGD("JSON decoder: %s '%.*s' is not an i32", __func__, len, start);
        return -EINVAL;
    }
    *val = val_i32;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_i64_json(struct thingset_msg* msg, int64_t *val)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    char *endptr = NULL;
    int64_t val_i64 = strtoll(start, &endptr, 10);
    if (endptr != (start + len)) {
        TS_LOGD("JSON decoder: %s '%.*s' is not an i64", __func__, len, start);
        return -EINVAL;
    }
    *val = val_i64;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_map_json(struct thingset_msg* msg, uint16_t *num_elements)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_OBJECT) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }
    *num_elements = size;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_map_end_json(struct thingset_msg* msg)
{
    return 0;
}

int ts_msg_pull_mem_json(struct thingset_msg* msg, const uint8_t **mem, uint16_t *mem_len)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_STRING) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    /** @todo Do in place conversion from base64 to bytes */
    TS_LOGD("JSON decoder: %s not supported", __func__);
    return -ENOTSUP;

    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_string_json(struct thingset_msg* msg, const char **s, uint16_t *s_len)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_STRING) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }
    *s = start;
    *s_len = len;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_u16_json(struct thingset_msg* msg, uint16_t *val)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    char *endptr = NULL;
    uint16_t val_u16 = (uint16_t)strtoul(start, &endptr, 10);
    if (endptr != (start + len)) {
        TS_LOGD("JSON decoder: %s '%.*s' is not an u16", __func__, len, start);
        return -EINVAL;
    }
    *val = val_u16;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_u32_json(struct thingset_msg* msg, uint32_t *val)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    char *endptr = NULL;
    uint32_t val_u32 = strtoul(start, &endptr, 10);
    if (endptr != (start + len)) {
        TS_LOGD("JSON decoder: %s '%.*s' is not an u32", __func__, len, start);
        return -EINVAL;
    }
    *val = val_u32;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

int ts_msg_pull_u64_json(struct thingset_msg* msg, uint64_t *val)
{
    TS_MSG_JSON_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    uint16_t type, size, len;
    const char *start;
    int ret = ts_jsmn_token_by_index(&scratchpad->jsmn, scratchpad->token_idx,
                                     &type, &size, &start, &len);
    if (ret != 0) {
        TS_LOGD("JSON decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (type != TS_JSMN_PRIMITIVE) {
        TS_LOGD("JSON decoder: %s with wrong type (%u)", __func__, (unsigned int)type);
        return -EINVAL;
    }

    char *endptr = NULL;
    uint64_t val_u64 = strtoull(start, &endptr, 10);
    if (endptr != (start + len)) {
        TS_LOGD("JSON decoder: %s '%.*s' is not an u64", __func__, len, start);
        return -EINVAL;
    }
    *val = val_u64;
    return ts_msg_json_dec_update(msg, __func__, start, len);
}

/* CBOR values */

int ts_msg_add_array_cbor(struct thingset_msg* msg, uint16_t num_elements)
{
    TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    if (scratchpad->current >= 2) {
        TS_LOGD("CBOR encoder: %s without more encoders", __func__);
        return -ENOMEM;
    }

    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    scratchpad->current++;
    struct CborEncoder *array_encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);

    CborError error = cbor_encoder_create_array(encoder, array_encoder, num_elements);
    if (error != CborNoError) {
        scratchpad->current--;
    }
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_array_end_cbor(struct thingset_msg* msg)
{
    if (ts_msg_scratchpad_tinycbor_enc_is_top(msg)) {
        /* Can not close on top level encoder */
        TS_LOGD("CBOR encoder: %s close while on top level encoder", __func__);
        return -ENOMEM;
    }

    TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    struct CborEncoder *array_encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    scratchpad->current--;
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);

    CborError error = cbor_encoder_close_container(encoder, array_encoder);

    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_bool_cbor(struct thingset_msg* msg, bool b)
{
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    CborError error = cbor_encode_boolean(encoder, b);
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_decfrac_cbor(struct thingset_msg* msg, int32_t mantissa, int16_t exponent)
{
    /* Tag */
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    CborError error = cbor_encode_tag(encoder, CborDecimalTag);
    int ret = ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
    if (ret != 0) {
        return ret;
    }

    /* Value */
    ret = ts_msg_add_array_cbor(msg, 2);
    if (ret != 0) {
        return ret;
    }
    ret = ts_msg_add_i16_cbor(msg, exponent);
    if (ret != 0) {
        return ret;
    }
    ret = ts_msg_add_i32_cbor(msg, mantissa);
    if (ret != 0) {
        return ret;
    }
    ret = ts_msg_add_array_end_cbor(msg);
    return ret;
}

int ts_msg_add_f32_cbor(struct thingset_msg* msg, float val, int precision)
{

    if (precision == 0) {
        // round to 0 digits: use int
#if TS_64BIT_TYPES_SUPPORT
        int64_t val_i64 = llroundf(val);
        return ts_msg_add_i64_cbor(msg, val_i64);
#else
        return ts_msg_add_i32_cbor(msg, lroundf(val));
#endif
    }
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    CborError error = cbor_encode_float(encoder, val);
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_i64_cbor(struct thingset_msg* msg, int64_t val)
{
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    CborError error = cbor_encode_int(encoder, val);
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_map_cbor(struct thingset_msg* msg, uint16_t num_elements)
{
    TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    if (scratchpad->current >= 2) {
        TS_LOGD("CBOR encoder: %s without more encoders", __func__);
        return -ENOMEM;
    }

    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    scratchpad->current++;
    struct CborEncoder *map_encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);

    CborError error = cbor_encoder_create_map(encoder, map_encoder, num_elements);
    if (error != CborNoError) {
        scratchpad->current--;
    }
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_map_end_cbor(struct thingset_msg* msg)
{
    if (ts_msg_scratchpad_tinycbor_enc_is_top(msg)) {
        /* Can not close on top level encoder */
        TS_LOGD("CBOR encoder: %s close while on top level encoder", __func__);
        return -ENOMEM;
    }

    TS_MSG_CBOR_ENC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    struct CborEncoder *map_encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    scratchpad->current--;
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);

    CborError error = cbor_encoder_close_container(encoder, map_encoder);

    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_mem_cbor(struct thingset_msg* msg, const uint8_t *mem, uint16_t len)
{
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    CborError error = cbor_encode_byte_string(encoder, mem, (size_t)len);
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_string_cbor(struct thingset_msg* msg, const char *s)
{
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    CborError error = cbor_encode_text_stringz(encoder, s);
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_undefined_cbor(struct thingset_msg* msg)
{
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    CborError error = cbor_encode_undefined(encoder);
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_add_u64_cbor(struct thingset_msg* msg, uint64_t val)
{
    struct CborEncoder *encoder = ts_msg_scratchpad_tinycbor_enc_current(msg);
    CborError error = cbor_encode_uint(encoder, val);
    return ts_msg_scratchpad_tinycbor_enc_update(msg, __func__, error);
}

int ts_msg_pull_type_cbor(struct thingset_msg* msg, CborType *cbor_type)
{
    CborType type = cbor_value_get_type(ts_msg_scratchpad_tinycbor_dec_current(msg));
    if (type == CborInvalidType) {
        TS_LOGD("CBOR decoder: %s %s", __func__, cbor_error_string(type));
        return -EINVAL;
    }
    TS_LOGD("CBOR decoder: %s %u", __func__, (unsigned int)type);
    *cbor_type = type;
    return 0;
}

int ts_msg_pull_array_cbor(struct thingset_msg* msg, uint16_t *num_elements)
{
    TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    if (scratchpad->current >= 2) {
        TS_LOGD("CBOR decoder: pull array but no more value iterator available");
        return -ENOMEM;
    }

    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if (cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: pull array while on end");
        return -ENOMEM;
    }
    if (!cbor_value_is_array(it)) {
        TS_LOGD("CBOR decoder: pull array with wrong type (%u)",
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    scratchpad->current++;
    struct CborValue *array_it = ts_msg_scratchpad_tinycbor_dec_current(msg);

    CborError error = cbor_value_enter_container(it, array_it);
    if (error != CborNoError) {
        scratchpad->current--;
        return -EINVAL;
    }

    size_t length;
    error = cbor_value_get_array_length(it, &length);
    if ((error != CborNoError) || (length > UINT16_MAX)) {
        scratchpad->current--;
        return -EINVAL;
    }
    *num_elements = (uint16_t)length;

    return ts_msg_scratchpad_tinycbor_dec_update(msg, false, __func__, CborNoError);
}

int ts_msg_pull_array_end_cbor(struct thingset_msg* msg)
{
    if (ts_msg_scratchpad_tinycbor_dec_is_top(msg)) {
        /* Can not close array on top level value iterator */
        TS_LOGD("CBOR decoder: %s while on top level value iterator", __func__);
        return -ENOMEM;
    }

    struct CborValue *array_it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if (!cbor_value_at_end(array_it)) {
        /* we did not pull all of the values */
        TS_LOGD("CBOR decoder: %s while value iterator still holds value(s)", __func__);
        return -EAGAIN;
    }

    TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->current--;
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);

    CborError error = cbor_value_leave_container(it, array_it);

    return ts_msg_scratchpad_tinycbor_dec_update(msg, false, __func__, error);
}

int ts_msg_pull_bool_cbor(struct thingset_msg* msg, bool *b)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_boolean(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    (void)cbor_value_get_boolean(it, b); /* can't fail */

    return ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, CborNoError);
}

int ts_msg_pull_decfrac_cbor(struct thingset_msg* msg, int32_t *mantissa, int16_t *exponent)
{
    /* Tag */
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if (cbor_value_is_tag(it)) {
        CborTag tag;
        (void)cbor_value_get_tag(it, &tag);
        if (tag != CborDecimalTag) {
            TS_LOGD("CBOR decoder: %s wrong tag (%u)", __func__, (unsigned int)tag);
            return -EINVAL;
        }
        (void)ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, CborNoError);
    }
    else {
        TS_LOGD("CBOR decoder: %s not tagged", __func__);
        if (!cbor_value_is_container(it)) {
            TS_LOGD("CBOR decoder: %s value is not a container", __func__);
            return -EINVAL;
        }
    }

    /* Value */
    uint16_t num_elements;
    int ret = ts_msg_pull_array_cbor(msg, &num_elements);
    if (ret != 0) {
        return ret;
    }
    if (num_elements != 2) {
        TS_LOGD("CBOR decoder: %s wrong number of elements (%u)", __func__,
                (unsigned int)num_elements);
        return -EINVAL;
    }
    int32_t val_mantissa;
    int16_t val_exponent;
    ret = ts_msg_pull_i16_cbor(msg, &val_exponent);
    if (ret != 0) {
        return ret;
    }
    ret = ts_msg_pull_i32_cbor(msg, &val_mantissa);
    if (ret != 0) {
        return ret;
    }
    ret = ts_msg_pull_array_end_cbor(msg);
    if (ret != 0) {
        return ret;
    }

    *mantissa = val_mantissa;
    *exponent = val_exponent;
    return 0;
}

int ts_msg_pull_f32_cbor(struct thingset_msg* msg, float *val)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if (cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s pull while at end", __func__);
        return -ENOMEM;
    }
    CborError error;
    if (cbor_value_is_float(it)) {
        error = cbor_value_get_float(it, val);
    }
    else if (cbor_value_is_integer(it)) {
        uint64_t val_uint64;
        error = cbor_value_get_uint64(it, &val_uint64);
        *val = (float)val_uint64;
    }
    else {
        TS_LOGD("CBOR decoder: pull f32 with wrong type (%u)",
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }
    return ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, error);
}

int ts_msg_pull_i16_cbor(struct thingset_msg* msg, int16_t *val)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_integer(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    int64_t val_i64;
    (void)cbor_value_get_int64(it, &val_i64); /* can't fail */
    if ((val_i64 < INT16_MIN) || (val_i64 > INT16_MAX)) {
        TS_LOGD("CBOR decoder: %s value exceeds range (%" PRIi64 ")", __func__, val_i64);
        return EINVAL;
    }
    *val = (int16_t)val_i64;
    return ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, CborNoError);
}

int ts_msg_pull_i32_cbor(struct thingset_msg* msg, int32_t *val)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_integer(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    int64_t val_i64;
    (void)cbor_value_get_int64(it, &val_i64); /* can't fail */
    if ((val_i64 < INT32_MIN) || (val_i64 > INT32_MAX)) {
        TS_LOGD("CBOR decoder: %s value exceeds range (%" PRIu64 ")", __func__, val_i64);
        return EINVAL;
    }
    *val = (int32_t)val_i64;
    return ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, CborNoError);
}

int ts_msg_pull_i64_cbor(struct thingset_msg* msg, int64_t *val)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_integer(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    (void)cbor_value_get_int64(it, val); /* can't fail */

    return ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, CborNoError);
}

int ts_msg_pull_map_cbor(struct thingset_msg* msg, uint16_t *num_elements)
{
    TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    if (scratchpad->current >= 2) {
        TS_LOGD("CBOR decoder: pull map but no more value iterator available");
        return -ENOMEM;
    }

    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if (cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: pull map while on end");
        return -ENOMEM;
    }
    if (!cbor_value_is_map(it)) {
        TS_LOGD("CBOR decoder: pull map with wrong type (%u)",
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    scratchpad->current++;
    struct CborValue *map_it = ts_msg_scratchpad_tinycbor_dec_current(msg);

    CborError error = cbor_value_enter_container(it, map_it);
    if (error != CborNoError) {
        scratchpad->current--;
        return -EINVAL;
    }

    size_t length;
    error = cbor_value_get_map_length(it, &length);
    if ((error != CborNoError) || (length > UINT16_MAX)) {
        scratchpad->current--;
        return -EINVAL;
    }
    *num_elements = (uint16_t)length;

    return ts_msg_scratchpad_tinycbor_dec_update(msg, false, __func__, CborNoError);
}

int ts_msg_pull_map_end_cbor(struct thingset_msg* msg)
{
    if (ts_msg_scratchpad_tinycbor_dec_is_top(msg)) {
        /* Can not close map on top level value iterator */
        TS_LOGD("CBOR decoder: %s while on top level value iterator", __func__);
        return -ENOMEM;
    }

    struct CborValue *map_it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if (!cbor_value_at_end(map_it)) {
        /* we did not pull all of the values */
        TS_LOGD("CBOR decoder: %s while value iterator still holds value(s)", __func__);
        return -EAGAIN;
    }

    TS_MSG_CBOR_DEC_SCRATCHPAD_PTR_INIT(scratchpad, msg);
    scratchpad->current--;
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);

    CborError error = cbor_value_leave_container(it, map_it);

    return ts_msg_scratchpad_tinycbor_dec_update(msg, false, __func__, error);
}

int ts_msg_pull_mem_cbor(struct thingset_msg* msg, const uint8_t **mem, uint16_t *len)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_byte_string(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }
    if (!cbor_value_is_length_known(it)) {
        TS_LOGD("CBOR decoder: %s with unknown length", __func__);
        return -EINVAL;
    }

    size_t val_len;
    (void)cbor_value_get_string_length(it, &val_len); /* error already checked */

    CborError error = cbor_value_advance(it);
    *mem = cbor_value_get_next_byte(it) - val_len;
    *len = (uint16_t)val_len;

    return ts_msg_scratchpad_tinycbor_dec_update(msg, false, __func__, error);
}

int ts_msg_pull_string_cbor(struct thingset_msg* msg, const char **s, uint16_t *len)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_text_string(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }
    if (!cbor_value_is_length_known(it)) {
        TS_LOGD("CBOR decoder: %s with unknown length", __func__);
        return -EINVAL;
    }

    size_t val_len;
    (void)cbor_value_get_string_length(it, &val_len); /* error already checked */

    CborError error = cbor_value_advance(it);
    *s = cbor_value_get_next_byte(it) - val_len;
    *len = (uint16_t)val_len;

    return ts_msg_scratchpad_tinycbor_dec_update(msg, false, __func__, error);
}

int ts_msg_pull_u16_cbor(struct thingset_msg* msg, uint16_t *val)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_unsigned_integer(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    uint64_t val_u64;
    (void)cbor_value_get_uint64(it, &val_u64); /* can't fail */
    if (val_u64 > UINT16_MAX) {
        TS_LOGD("CBOR decoder: %s value exceeds range (%" PRIu64 ")", __func__, val_u64);
        return EINVAL;
    }
    *val = (uint16_t)val_u64;
    return ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, CborNoError);
}

int ts_msg_pull_u32_cbor(struct thingset_msg* msg, uint32_t *val)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_unsigned_integer(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    uint64_t val_u64;
    (void)cbor_value_get_uint64(it, &val_u64); /* can't fail */
    if (val_u64 > UINT32_MAX) {
        TS_LOGD("CBOR decoder: %s value exceeds range (%" PRIu64 ")", __func__, val_u64);
        return EINVAL;
    }
    *val = (uint32_t)val_u64;
    return ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, CborNoError);
}

int ts_msg_pull_u64_cbor(struct thingset_msg* msg, uint64_t *val)
{
    struct CborValue *it = ts_msg_scratchpad_tinycbor_dec_current(msg);
    if ((ts_msg_len(msg) == 0) || cbor_value_at_end(it)) {
        TS_LOGD("CBOR decoder: %s while at end", __func__);
        return -ENOMEM;
    }
    if (!cbor_value_is_unsigned_integer(it)) {
        TS_LOGD("CBOR decoder: %s with wrong type (%u)", __func__,
                (unsigned int)cbor_value_get_type(it));
        return -EINVAL;
    }

    (void)cbor_value_get_uint64(it, val); /* can't fail */

    return ts_msg_scratchpad_tinycbor_dec_update(msg, true, __func__, CborNoError);
}
