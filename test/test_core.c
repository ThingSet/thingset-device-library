/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include "test.h"

char test_log_buf[300];
uint8_t test_req_buf[TEST_REQ_BUFFER_LEN];
uint8_t test_resp_buf[TEST_RESP_BUFFER_LEN];

bool conf_callback_called;
bool dummy_called_flag;
struct ts_array_info pub_serial_array;

void dummy(void)
{
    dummy_called_flag = 1;
}

void conf_callback(void)
{
    conf_callback_called = 1;
}

void reset_function()
{
    LOG_DBG("Reset function called!\n");
}

void auth_function()
{
    const char pass_exp[] = "expert123";
    const char pass_mkr[] = "maker456";

    if (strlen(pass_exp) == strlen(auth_password) &&
        strncmp(auth_password, pass_exp, strlen(pass_exp)) == 0)
    {
        thingset_authorisation_set(&test_ts_core, TS_EXP_RW | TS_USR_RW);
    }
    else if (strlen(pass_mkr) == strlen(auth_password) &&
        strncmp(auth_password, pass_mkr, strlen(pass_mkr)) == 0)
    {
        thingset_authorisation_set(&test_ts_core, TS_MKR_RW | TS_EXP_RW | TS_USR_RW);
    }
    else {
        thingset_authorisation_set(&test_ts_core, TS_USR_RW);
    }

    LOG_DBG("Auth function called, password: %s\n", auth_password);
}

int _hex2bin(uint8_t *bin, size_t bin_size, const char *hex)
{
    int len = strlen(hex);
    unsigned int pos = 0;
    for (int i = 0; i < len; i += 3) {
        if (pos < bin_size) {
            bin[pos++] = (char)strtoul(&hex[i], NULL, 16);
        }
        else {
            return 0;
        }
    }
    return pos;
}

// determines the size of a cbor data item starting at given pointer
#define CBOR_TYPE_MASK          0xE0    /* top 3 bits */
#define CBOR_INFO_MASK          0x1F    /* low 5 bits */

/* Jump Table for Initial Byte (cf. table 5) */
#define CBOR_UINT       0x00            /* type 0 */
#define CBOR_NEGINT     0x20            /* type 1 */
#define CBOR_BYTES      0x40            /* type 2 */
#define CBOR_TEXT       0x60            /* type 3 */
#define CBOR_ARRAY      0x80            /* type 4 */
#define CBOR_MAP        0xA0            /* type 5 */
#define CBOR_TAG        0xC0            /* type 6 */
#define CBOR_7          0xE0            /* type 7 (float and other types) */

#define CBOR_NUM_MAX            23      /* maximum number that can be directl encoded */

/* Major types (cf. section 2.1) */
/* Major type 0: Unsigned integers */
#define CBOR_UINT8_FOLLOWS      24      /* 0x18 */
#define CBOR_UINT16_FOLLOWS     25      /* 0x19 */
#define CBOR_UINT32_FOLLOWS     26      /* 0x1a */
#define CBOR_UINT64_FOLLOWS     27      /* 0x1b */

/* Indefinite Lengths for Some Major types (cf. section 2.2) */
#define CBOR_VAR_FOLLOWS        31      /* 0x1f */

/* Major type 6: Semantic tagging */
#define CBOR_DATETIME_STRING_FOLLOWS        0
#define CBOR_DATETIME_EPOCH_FOLLOWS         1
#define CBOR_DECFRAC_ARRAY_FOLLOWS          4

/* Major type 7: Float and other types */
#define CBOR_FALSE      (CBOR_7 | 20)
#define CBOR_TRUE       (CBOR_7 | 21)
#define CBOR_NULL       (CBOR_7 | 22)
#define CBOR_UNDEFINED  (CBOR_7 | 23)
#define CBOR_SIMPLE     (CBOR_7 | 24)
#define CBOR_FLOAT16    (CBOR_7 | 25)
#define CBOR_FLOAT32    (CBOR_7 | 26)
#define CBOR_FLOAT64    (CBOR_7 | 27)
#define CBOR_BREAK      (CBOR_7 | 31)

static int _cbor_size(const uint8_t *data)
{
    uint8_t type = data[0] & CBOR_TYPE_MASK;
    uint8_t info = data[0] & CBOR_INFO_MASK;

    if (type == CBOR_UINT || type == CBOR_NEGINT) {
        if (info <= CBOR_NUM_MAX)
            return 1;
        switch (info) {
        case CBOR_UINT8_FOLLOWS:
            return 2;
        case CBOR_UINT16_FOLLOWS:
            return 3;
        case CBOR_UINT32_FOLLOWS:
            return 5;
        case CBOR_UINT64_FOLLOWS:
            return 9;
        }
    }
    else if (type == CBOR_BYTES || type == CBOR_TEXT) {
        if (info <= CBOR_NUM_MAX) {
            return info + 1;
        }
        else {
            if (info == CBOR_UINT8_FOLLOWS)
                return 1 + data[1];
            else if (info == CBOR_UINT16_FOLLOWS)
                return 1 + (data[1] << 8 | data[2]);
            else
                return 0;   // longer string / byte array not supported
        }
    }
#if TS_CONFIG_DECFRAC_TYPE_SUPPORT
    else if (type == CBOR_TAG && info == CBOR_DECFRAC_ARRAY_FOLLOWS) {
        int pos = 2;
        pos += _cbor_size(&data[pos]);     // exponent
        pos += _cbor_size(&data[pos]);     // mantissa
        return pos;
    }
#endif
    else if (type == CBOR_7) {
        switch (data[0]) {
        case CBOR_FALSE:
        case CBOR_TRUE:
            return 1;
            break;
        case CBOR_FLOAT32:
            return 5;
            break;
        case CBOR_FLOAT64:
            return 9;
            break;
        }
    }

    return 0;   // float16, arrays, maps, tagged types, etc. curently not supported
}

static int _bin2hex(char *hex, size_t hex_size, const uint8_t *bin, size_t bin_size)
{
    size_t bin_idx, hex_idx;

    for (bin_idx = hex_idx = 0; bin_idx < bin_size; bin_idx++) {
        uint8_t b = bin[bin_idx] >> 4;
        if (hex_idx >= hex_size) {
            return -1;
        }
        hex[hex_idx++] = (char) (87 + b + (((b - 10) >> 31) & -39));
        b = bin[bin_idx] & 0xf;
        if (hex_idx >= hex_size) {
            return -1;
        }
        hex[hex_idx++] = (char) (87 + b + (((b - 10) >> 31) & -39));
        if (hex_idx < hex_size) {
            hex[hex_idx++] = ' ';
        }
    }
    if (hex_idx >= hex_size) {
        return -1;
    }
    hex[hex_idx] = '\0';

    return hex_idx;
}

void assert_bin_resp(const uint8_t *resp_buf, int resp_len, const char *exp_hex, const char *msg)
{
    char resp_hex[100];
    uint8_t exp_b[100];

    int exp_len = _hex2bin(exp_b, sizeof(exp_b), exp_hex);
    (void)_bin2hex(resp_hex, sizeof(resp_hex), resp_buf, resp_len);

    TEST_ASSERT_EQUAL_MESSAGE(exp_len, resp_len, msg);
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(exp_b, resp_buf, exp_len, msg);
}

void assert_bin_req(const uint8_t *req_b, int req_len, const char *exp_hex, const char* msg)
{
    int resp_len = thingset_process_buf(&test_ts_core, req_b, req_len, test_resp_buf, TEST_RESP_BUFFER_LEN);

    assert_bin_resp(test_resp_buf, resp_len, exp_hex, msg);
}

void assert_bin_req_exp_bin(const uint8_t *req_b, int req_len, const uint8_t *exp_b, int exp_len, const char* msg)
{
    char exp_hex[exp_len * 3 + 1];
    (void)_bin2hex(exp_hex, sizeof(exp_hex), exp_b, exp_len);

    assert_bin_req(req_b, req_len, exp_hex, msg);
}

void assert_bin_req_hex(const char *req_hex, const char *exp_hex, const char *msg)
{
    int req_len = _hex2bin(test_req_buf, TEST_REQ_BUFFER_LEN, req_hex);

    TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(TEST_REQ_BUFFER_LEN, req_len, msg);

    assert_bin_req(test_req_buf, req_len, exp_hex, msg);
}

void assert_txt_resp(int exp_len, const char *exp_s, const char *msg)
{
    int resp_buf_len = strlen((char *)test_resp_buf);

    TEST_ASSERT_EQUAL_STRING_MESSAGE(exp_s, (char *)test_resp_buf, msg);
    TEST_ASSERT_EQUAL_MESSAGE(exp_len, resp_buf_len, msg);
}

void assert_txt_req(const char *req_s, const char *exp_s, const char *msg)
{
    int req_len = strlen(req_s);
    TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(TEST_REQ_BUFFER_LEN - 1, req_len, msg);

    strncpy((char *)test_req_buf, req_s, req_len);
    int resp_len = thingset_process_buf(&test_ts_core, test_req_buf, req_len, test_resp_buf,
                                        TEST_RESP_BUFFER_LEN);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, resp_len, msg);

    assert_txt_resp(resp_len, exp_s, msg);
}

static void _txt_patch(char const *name, char const *value, const char *msg)
{
    int req_len = snprintf((char *)test_req_buf, TEST_REQ_BUFFER_LEN, "=conf {\"%s\":%s}", name, value);
    int resp_len = thingset_process_buf(&test_ts_core, test_req_buf, req_len, test_resp_buf,
                                        TEST_RESP_BUFFER_LEN);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, resp_len, msg);

    int resp_buf_len = strlen((char *)test_resp_buf);

    TEST_ASSERT_EQUAL_MESSAGE(resp_len, resp_buf_len, msg);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(":84 Changed.", (char *)test_resp_buf, msg);
}

static int _txt_fetch(char const *name, char *value_read, const char *msg)
{
    int req_len = snprintf((char *)test_req_buf, TEST_REQ_BUFFER_LEN, "?conf \"%s\"", name);
    int resp_len = thingset_process_buf(&test_ts_core, test_req_buf, req_len, test_resp_buf,
                                        TEST_RESP_BUFFER_LEN);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, resp_len, msg);

    int resp_buf_len = strlen((char *)test_resp_buf);

    TEST_ASSERT_EQUAL_MESSAGE(resp_len, resp_buf_len, msg);

    int pos_dot = strchr((char *)test_resp_buf, '.') - (char *)test_resp_buf + 1;
    char buf[100];
    strncpy(buf, (char *)test_resp_buf, pos_dot);
    buf[pos_dot] = '\0';

    TEST_ASSERT_EQUAL_STRING_MESSAGE(":85 Content.", buf, msg);

    return snprintf(value_read, strlen((char *)test_resp_buf) - pos_dot, "%s", test_resp_buf + pos_dot + 1);
}

// returns length of read value
static int _bin_fetch(uint16_t id, char *value_read, const char *msg)
{
    uint8_t req[] = {
        TS_FETCH,
        0x18, ID_CONF,
        0x19, (uint8_t)(id >> 8), (uint8_t)id
    };
    int ret = thingset_process_buf(&test_ts_core, req, sizeof(req), test_resp_buf,
                                   TEST_RESP_BUFFER_LEN);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, ret, msg);

    TEST_ASSERT_EQUAL_HEX8_MESSAGE(TS_STATUS_CONTENT, test_resp_buf[0], msg);

    int value_len = _cbor_size((uint8_t*)test_resp_buf + 1);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, value_len, msg);

    memcpy(value_read, test_resp_buf + 1, value_len);
    return value_len;
}

// returns length of read value
static void _bin_patch(uint16_t id, char *value, const char *msg)
{
    uint8_t req[100] = {
        TS_PATCH,
        0x18, ID_CONF,
        0xA1,
        0x19, (uint8_t)(id >> 8), (uint8_t)id
    };
    unsigned int len = _cbor_size((uint8_t*)value);
    TEST_ASSERT_LESS_THAN_size_t_MESSAGE(sizeof(req) - 7, len, msg);

    memcpy(req + 7, value, len);
    int ret = thingset_process_buf(&test_ts_core, req, len + 7, test_resp_buf,
                                   TEST_RESP_BUFFER_LEN);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, ret, msg);

    TEST_ASSERT_EQUAL_HEX8_MESSAGE(TS_STATUS_CHANGED, test_resp_buf[0], msg);
}

void assert_json2cbor(char const *name, char const *json_value, uint16_t id,
                      const char *const cbor_value_hex, const char *msg)
{
    char buf[100];  // temporary data storage (JSON or CBOR)
    uint8_t cbor_value[100];
    int len = _hex2bin(cbor_value, sizeof(cbor_value), (char *)cbor_value_hex);

    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, len, msg);
    TEST_ASSERT_LESS_THAN_INT_MESSAGE(100, len, msg);

    _txt_patch(name, json_value, msg);
    len = _bin_fetch(id, buf, msg);

    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, len, msg);
    TEST_ASSERT_LESS_THAN_INT_MESSAGE(100, len, msg);

    //TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(&cbor_value[0], &buf[0], len, msg);
}

void assert_cbor2json(char const *name, char const *json_value, uint16_t id, char const *cbor_value_hex, const char *msg)
{
    char buf[100];  // temporary data storage (JSON or CBOR)
    char cbor_value[100];
    _hex2bin((uint8_t *)cbor_value, sizeof(cbor_value), (char *)cbor_value_hex);

    _bin_patch(id, cbor_value, msg);
    _txt_fetch(name, buf, msg);

    TEST_ASSERT_EQUAL_STRING_MESSAGE(json_value, buf, msg);
}

void assert_msg_add_response_get_cbor(struct thingset_msg *response, struct thingset_msg *request,
                                      const char *object_path, int expected_ret,
                                      const char *expected_str, const char *assert_msg)
{
    int ret;
    thingset_oref_t object_oref;
    char tc_desc[300];

    /* Prepare assert message */
    int len = snprintf(&tc_desc[0], sizeof(tc_desc) - 1, "%s: ", assert_msg);

    LOG_DBG("%s", &tc_desc[sizeof("Assertion triggered by")]);

    ts_obj_db_oref_init(TEST_DB_ID_INSTANCE, &object_oref);

    thingset_msg_reset(response);
    TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(response), &tc_desc[0]);

    if (object_path == NULL) {
        ts_obj_db_oref_init(TEST_DB_ID_INSTANCE, &object_oref);
    }
    else {
        ret = ts_obj_db_oref_by_path(TEST_DB_ID_INSTANCE, object_path,
                                    (size_t)strlen(object_path),
                                        &object_oref);
        TEST_ASSERT_EQUAL_MESSAGE(0, ret, &tc_desc[0]);
    }

    ret = ts_msg_add_response_get_cbor(response, object_oref, request);
    ts_msg_log(response, &test_log_buf[0], sizeof(test_log_buf));

    /* Complete assert message */
    const char *object_name = "<none>";
    ts_obj_id_t object_id = 0;
    ts_obj_id_t object_parent_id = 0;
    uint8_t object_type = 0;
    if (ts_obj_db_oref_is_valid(object_oref)) {
        object_id = ts_obj_id(object_oref);
        object_name = ts_obj_name(object_oref);
        object_parent_id = ts_obj_parent_id(object_oref);
        object_type = ts_obj_type(object_oref);
    }
    snprintf(&tc_desc[len], sizeof(tc_desc) - len - 1,
        " - '%s', id: %u, type: %u, parent: %u -> %s", object_name, (unsigned int)object_id,
        (unsigned int)object_type, (unsigned int) object_parent_id, &test_log_buf[0]);

    TEST_ASSERT_EQUAL_MESSAGE(expected_ret, ret, &tc_desc[0]);
    if (expected_str != NULL) {
        TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_str, &test_log_buf[0], &tc_desc[0]);
    }
    else {
        TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(response), &tc_desc[0]);
    }
}

void assert_msg_fetch_cbor(struct thingset_msg *request, struct thingset_msg *response,
                           const char *object_path,
                           bool test_object_id,
                           bool test_object_ids, uint16_t object_count, const char **object_names,
                           /* returns of ts_msg_add_request_fetch_cbor */
                           int expected_ret_add_request, const char *expected_str_add_request,
                           /* returns of ts_msg_pull_request_cbor */
                           int expected_ret_pull_request, uint8_t expected_status_id_pull_request,
                           /* returns of ts_msg_add_response_fetch_cbor */
                           int expected_ret_add_response, const char *expected_str_add_response,
                           const char *assert_msg)
{
    int ret;
    thingset_oref_t object_oref;
    ts_obj_id_t object_id;
    char tc_desc[300];
    ts_obj_id_t names_object_ids[object_count];
    struct ts_msg_stat expected_status_pull_request;

    /* Prepare assert message */
    int len = snprintf(&tc_desc[0], sizeof(tc_desc) - 1, "%s: ", assert_msg);

    LOG_DBG("%s", &tc_desc[sizeof("Assertion triggered by")]);

    expected_status_pull_request.type = TS_MSG_TYPE_REQUEST;
    expected_status_pull_request.code = expected_status_id_pull_request;

    thingset_msg_reset(request);
    TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(request), &tc_desc[0]);
    thingset_msg_reset(response);
    TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(response), &tc_desc[0]);

    /*
     * Create fetch request
     * --------------------
     */
    if (object_path == NULL) {
        ts_obj_db_oref_init(TEST_DB_ID_INSTANCE, &object_oref);
    }
    else {
        ret = ts_obj_db_oref_by_path(TEST_DB_ID_INSTANCE, object_path,
                                        (size_t)strlen(object_path),
                                        &object_oref);
        TEST_ASSERT_EQUAL_MESSAGE(0, ret, &tc_desc[0]);
    }
    if (test_object_id) {
        TEST_ASSERT_TRUE_MESSAGE(ts_obj_db_oref_is_valid(object_oref), &tc_desc[0]);
        object_id = ts_obj_id(object_oref);
        object_path = 0;
    }
    else {
        object_id = 0;
    }
    ts_obj_id_t *object_ids = NULL;
    if (test_object_ids) {
        for (uint16_t j = 0; j < object_count; j++) {
            const char *name = object_names[j];
            size_t name_len = strlen(name);
            int32_t parent_id = -1;
            if (ts_obj_db_oref_is_valid(object_oref)) {
                parent_id = (int32_t)ts_obj_id(object_oref);
            }
            thingset_oref_t fetch_oref;
            ret = ts_obj_db_oref_by_name(TEST_DB_ID_INSTANCE, name, name_len, parent_id,
                                         &fetch_oref);
            TEST_ASSERT_EQUAL_MESSAGE(0, ret, &tc_desc[0]);
            names_object_ids[j] = ts_obj_id(fetch_oref);
        }
        object_ids = &names_object_ids[0];
        object_names = NULL;
    }
    ret = ts_msg_add_request_fetch_cbor(request, object_id, object_path,
                                        object_count, object_ids, object_names);
    ts_msg_log(request, &test_log_buf[0], sizeof(test_log_buf));

    /* Complete assert message */
    const char *object_name = "<none>";
    ts_obj_id_t object_parent_id = 0;
    uint8_t object_type = 0;
    if (ts_obj_db_oref_is_valid(object_oref)) {
        object_id = ts_obj_id(object_oref);
        object_name = ts_obj_name(object_oref);
        object_parent_id = ts_obj_parent_id(object_oref);
        object_type = ts_obj_type(object_oref);
    }
    snprintf(&tc_desc[len], sizeof(tc_desc) - len - 1,
        " - '%s', id: %u, type: %u, parent: %u -> %s", object_name, (unsigned int)object_id,
        (unsigned int)object_type, (unsigned int)object_parent_id, &test_log_buf[0]);

    TEST_ASSERT_EQUAL_MESSAGE(expected_ret_add_request, ret, &tc_desc[0]);
    if (expected_str_add_request != NULL) {
        TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_str_add_request,
                                         &test_log_buf[0], &tc_desc[0]);
    }
    else {
        TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(request), &tc_desc[0]);
    }

    /*
     * Pull fetch request
     * ------------------
     */

    /* Prepare assert message (again) */
    len = snprintf(&tc_desc[0], sizeof(tc_desc) - 1, "%s: ", assert_msg);

    thingset_oref_t request_oref = { .db_id = TEST_DB_ID_INSTANCE };
    ts_msg_auth_set(request, thingset_authorisation(&test_ts_instance));
    ret = ts_msg_pull_request_cbor(request, &request_oref);

    /* Complete assert message */
    object_name = "<none>";
    if (ts_obj_db_oref_is_valid(request_oref)) {
        object_id = ts_obj_id(request_oref);
        object_name = ts_obj_name(request_oref);
        object_parent_id = ts_obj_parent_id(request_oref);
        object_type = ts_obj_type(request_oref);
    }
    snprintf(&tc_desc[len], sizeof(tc_desc) - len - 1,
        " - '%s', id: %u, type: %u, parent: %u, status: 0x%02x",
        object_name, (unsigned int)object_id, (unsigned int)object_type,
        (unsigned int)object_parent_id, (unsigned int)ts_msg_status_code(request));

    TEST_ASSERT_EQUAL_MESSAGE(expected_ret_pull_request, ret, &tc_desc[0]);
    TEST_ASSERT_EQUAL_MESSAGE(expected_status_pull_request.type, ts_msg_status_type(request),
                              &tc_desc[0]);
    TEST_ASSERT_EQUAL_MESSAGE(expected_status_pull_request.code, ts_msg_status_code(request),
                              &tc_desc[0]);
    if (test_object_id) {
        TEST_ASSERT_EQUAL_MESSAGE(object_id, ts_obj_id(object_oref), &tc_desc[0]);
    }
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(thingset_authorisation(&test_ts_instance),
                                     ts_msg_auth(request), &tc_desc[0]);

    /*
     * Create fetch response
     * ---------------------
     */

    /* Prepare assert message (again) */
    len = snprintf(&tc_desc[0], sizeof(tc_desc) - 1, "%s: ", assert_msg);

    ret = ts_msg_add_response_fetch_cbor(response, request_oref, request);
    ts_msg_log(response, &test_log_buf[0], sizeof(test_log_buf));

    /* Complete assert message */
    object_name = "<none>";
    if (ts_obj_db_oref_is_valid(request_oref)) {
        object_id = ts_obj_id(request_oref);
        object_name = ts_obj_name(request_oref);
        object_parent_id = ts_obj_parent_id(request_oref);
        object_type = ts_obj_type(request_oref);
    }
    snprintf(&tc_desc[len], sizeof(tc_desc) - len - 1,
        " - '%s', id: %u, type: %u, parent: %u, status: %u -> %s",
        object_name, (unsigned int)object_id, (unsigned int)object_type,
        (unsigned int)object_parent_id, (unsigned int)ts_msg_status_code(request),
        &test_log_buf[0]);

    TEST_ASSERT_EQUAL_MESSAGE(expected_ret_add_response, ret, &tc_desc[0]);
    if (expected_str_add_response != NULL) {
        TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_str_add_response, &test_log_buf[0], &tc_desc[0]);
    }
    else {
        TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(response), &tc_desc[0]);
    }
}

void assert_msg_add_response_get_json(struct thingset_msg *response, struct thingset_msg *request,
                                      ts_obj_id_t obj_id, int expected_ret,
                                      const char *expected_str, const char *assert_msg)
{
    int ret;
    thingset_oref_t object_oref;
    char tc_desc[300];

    /* Prepare assert message */
    int len = snprintf(&tc_desc[0], sizeof(tc_desc) - 1, "%s: ", assert_msg);

    LOG_DBG("%s", &tc_desc[sizeof("Assertion triggered by")]);

    ts_obj_db_oref_init(TEST_DB_ID_INSTANCE, &object_oref);

    thingset_msg_reset(response);
    TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(response), &tc_desc[0]);

    ret = ts_obj_db_oref_by_id(TEST_DB_ID_INSTANCE, obj_id, &object_oref );
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, &tc_desc[0]);

    ret = ts_msg_add_response_get_json(response, object_oref, request);

    /* Complete assert message */
    ts_msg_log(response, &test_log_buf[0], sizeof(test_log_buf));
    snprintf(&tc_desc[len], sizeof(tc_desc) - len - 1,
                " -> ret: %d, oref: %u, name: '%s', >%s<",
                ret, (unsigned int)object_oref.db_oid, ts_obj_name(object_oref),
                &test_log_buf[0]);

    TEST_ASSERT_EQUAL_MESSAGE(expected_ret, ret, &tc_desc[0]);
    TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(expected_str, ts_msg_data(response),
                                         ts_msg_len(response), &tc_desc[0]);
    TEST_ASSERT_EQUAL_MESSAGE(strlen(expected_str), ts_msg_len(response), &tc_desc[0]);
}
