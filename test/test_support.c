/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021..2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include "test.h"

/*
 * Test environment
 * ----------------
 */

char test_assert_buffer[TEST_ASSERT_BUFFER_SIZE];
const char *test_file_current = "<unset>";
const char *test_func_current = "<unset>";

char test_log_buf[TEST_LOG_BUFFER_SIZE];
uint8_t test_req_buf[TEST_REQ_BUFFER_LEN];
uint8_t test_resp_buf[TEST_RESP_BUFFER_LEN];
struct ts_array_info pub_serial_array;

/*
 * Test functions and flags used for core variant ThingSet context
 * ---------------------------------------------------------------
 */

bool test_core_conf_callback_called;
bool test_core_dummy_called;

void test_core_dummy_function(void)
{
    test_core_dummy_called = 1;
}

void test_core_conf_callback(void)
{
    test_core_conf_callback_called = 1;
}

void test_core_reset_function(void)
{
    TS_LOGD("Core context reset function called!\n");
}

void test_core_auth_function(void)
{
    const char pass_exp[] = "expert123";
    const char pass_mkr[] = "maker456";

    if (strlen(pass_exp) == strlen(auth_password) &&
        strncmp(auth_password, pass_exp, strlen(pass_exp)) == 0)
    {
        thingset_authorisation_set(TEST_CORE_LOCID, TS_EXP_RW | TS_USR_RW);
    }
    else if (strlen(pass_mkr) == strlen(auth_password) &&
        strncmp(auth_password, pass_mkr, strlen(pass_mkr)) == 0)
    {
        thingset_authorisation_set(TEST_CORE_LOCID, TS_MKR_RW | TS_EXP_RW | TS_USR_RW);
    }
    else {
        thingset_authorisation_set(TEST_CORE_LOCID, TS_USR_RW);
    }

    TS_LOGD("Core context auth function called, password: %s\n", auth_password);
}

/*
 * Test functions and flags used for communication variant ThingSet context
 * ------------------------------------------------------------------------
 */

bool test_instance_conf_callback_called;
bool test_instance_dummy_called;

void test_instance_dummy_function(void)
{
    test_instance_dummy_called = 1;
}

void test_instance_conf_callback(void)
{
    test_instance_conf_callback_called = 1;
}

void test_instance_reset_function()
{
    TS_LOGD("Instance context reset function called!\n");
}

void test_instance_auth_function()
{
    const char pass_exp[] = "expert123";
    const char pass_mkr[] = "maker456";

    if (strlen(pass_exp) == strlen(auth_password) &&
        strncmp(auth_password, pass_exp, strlen(pass_exp)) == 0)
    {
        thingset_authorisation_set(TEST_INSTANCE_LOCID, TS_EXP_MASK | TS_USR_MASK);
    }
    else if (strlen(pass_mkr) == strlen(auth_password) &&
        strncmp(auth_password, pass_mkr, strlen(pass_mkr)) == 0)
    {
        thingset_authorisation_set(TEST_INSTANCE_LOCID, TS_MKR_MASK | TS_USR_MASK);
    }
    else {
        thingset_authorisation_set(TEST_INSTANCE_LOCID, TS_USR_MASK);
    }

    TS_LOGD("Instance context auth function called, password: %s\n", auth_password);
}

void test_neighbour_reset_function()
{
    TS_LOGD("Neighbour context reset function called!\n");
}

/*
 * Helpers and complex assert functions
 * ------------------------------------
 */

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

void assert_bin_resp(const uint8_t *resp_buf, int resp_len, const char *exp_hex,
                     unsigned int assert_line, const char *assert_msg)
{
    char resp_hex[100];
    uint8_t exp_b[100];

    int exp_len = _hex2bin(exp_b, sizeof(exp_b), exp_hex);
    (void)_bin2hex(resp_hex, sizeof(resp_hex), resp_buf, resp_len);

    UNITY_TEST_ASSERT_EQUAL_INT(exp_len, resp_len, assert_line, assert_msg);
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(exp_b, resp_buf, exp_len, assert_msg);
}

void assert_bin_req(const uint8_t *req_b, int req_len, const char *exp_hex,
                    unsigned int assert_line, const char *assert_msg)
{
    int resp_len = thingset_process_buf(TEST_CORE_LOCID, req_b, req_len, test_resp_buf,
                                        TEST_RESP_BUFFER_LEN);

    assert_bin_resp(test_resp_buf, resp_len, exp_hex, assert_line, assert_msg);
}

void assert_bin_req_exp_bin(const uint8_t *req_b, int req_len, const uint8_t *exp_b, int exp_len,
                            unsigned int assert_line, const char *assert_msg)
{
    char exp_hex[exp_len * 3 + 1];
    (void)_bin2hex(exp_hex, sizeof(exp_hex), exp_b, exp_len);

    assert_bin_req(req_b, req_len, exp_hex, assert_line, assert_msg);
}

void assert_bin_req_hex(const char *req_hex, const char *exp_hex,
                        unsigned int assert_line, const char *assert_msg)
{
    int req_len = _hex2bin(test_req_buf, TEST_REQ_BUFFER_LEN, req_hex);

    UNITY_TEST_ASSERT_SMALLER_OR_EQUAL_UINT(TEST_REQ_BUFFER_LEN, req_len, assert_line,
                                            assert_msg);

    assert_bin_req(test_req_buf, req_len, exp_hex, assert_line, assert_msg);
}

void assert_txt_resp(int exp_len, const char *exp_s, const char *msg)
{
    int resp_buf_len = strnlen((char *)test_resp_buf, sizeof(test_resp_buf));

    TEST_ASSERT_EQUAL_STRING_MESSAGE(exp_s, (char *)test_resp_buf, msg);
    TEST_ASSERT_EQUAL_MESSAGE(exp_len, resp_buf_len, msg);
}

void assert_txt_req(const char *req_s, const char *exp_s, const char *msg)
{
    int req_len = strlen(req_s);
    TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(TEST_REQ_BUFFER_LEN - 1, req_len, msg);

    strncpy((char *)test_req_buf, req_s, req_len);
    int resp_len = thingset_process_buf(TEST_CORE_LOCID, test_req_buf, req_len, test_resp_buf,
                                        TEST_RESP_BUFFER_LEN);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, resp_len, msg);

    assert_txt_resp(resp_len, exp_s, msg);
}

static void _txt_patch(char const *name, char const *value, const char *msg)
{
    int req_len = snprintf((char *)test_req_buf, TEST_REQ_BUFFER_LEN, "=conf {\"%s\":%s}", name, value);
    int resp_len = thingset_process_buf(TEST_CORE_LOCID, test_req_buf, req_len, test_resp_buf,
                                        TEST_RESP_BUFFER_LEN);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, resp_len, msg);

    int resp_buf_len = strnlen((char *)test_resp_buf, sizeof(test_resp_buf));

    TEST_ASSERT_EQUAL_MESSAGE(resp_len, resp_buf_len, msg);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(":84 Changed.", (char *)test_resp_buf, msg);
}

static int _txt_fetch(char const *name, char *value_read, const char *msg)
{
    int req_len = snprintf((char *)test_req_buf, TEST_REQ_BUFFER_LEN, "?conf \"%s\"", name);
    int resp_len = thingset_process_buf(TEST_CORE_LOCID, test_req_buf, req_len, test_resp_buf,
                                        TEST_RESP_BUFFER_LEN);
    TEST_ASSERT_GREATER_OR_EQUAL_INT_MESSAGE(0, resp_len, msg);

    int resp_buf_len = strnlen((char *)test_resp_buf, sizeof(test_resp_buf));

    TEST_ASSERT_EQUAL_MESSAGE(resp_len, resp_buf_len, msg);

    int pos_dot = strchr((char *)test_resp_buf, '.') - (char *)test_resp_buf + 1;
    char buf[100];
    strncpy(buf, (char *)test_resp_buf, pos_dot);
    buf[pos_dot] = '\0';

    TEST_ASSERT_EQUAL_STRING_MESSAGE(":85 Content.", buf, msg);

    return snprintf(value_read, strnlen((char *)test_resp_buf, sizeof(test_resp_buf)) - pos_dot, "%s", test_resp_buf + pos_dot + 1);
}

// returns length of read value
static int _bin_fetch(uint16_t id, char *value_read, const char *msg)
{
    uint8_t req[] = {
        TS_FETCH,
        0x18, ID_CONF,
        0x19, (uint8_t)(id >> 8), (uint8_t)id
    };
    int ret = thingset_process_buf(TEST_CORE_LOCID, req, sizeof(req), test_resp_buf,
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
    int ret = thingset_process_buf(TEST_CORE_LOCID, req, len + 7, test_resp_buf,
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
                                      const char *expected_str,
                                      unsigned int assert_line, const char *assert_msg)
{
    int ret;
    thingset_oref_t object_oref = ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID));
    char tc_desc[300];

    /* Prepare assert message */
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1, "%s\nAt %s#%d", assert_msg, __FILE__, __LINE__);

    TS_LOGD("%s", &tc_desc[sizeof("Assertion triggered by")]);

    thingset_msg_reset(response);
    UNITY_TEST_ASSERT_EQUAL_UINT16(0, ts_msg_len(response), assert_line, &tc_desc[0]);

    if (object_path != NULL) {
        ret = ts_obj_by_path(ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID)), object_path,
                             (size_t)strlen(object_path), &object_oref);
        UNITY_TEST_ASSERT_EQUAL_INT(0, ret, assert_line, &tc_desc[0]);
    }

    ret = ts_msg_add_response_get_cbor(response, object_oref, request);
    ts_msg_log(response, &test_log_buf[0], sizeof(test_log_buf));

    /* Prepare assert message (again) */
    const char *object_name = "<none>";
    ts_obj_id_t object_id = 0;
    ts_obj_id_t object_parent_id = 0;
    uint8_t object_type = 0;
    if (ts_obj_db_oref_is_object(object_oref)) {
        object_id = ts_obj_id(object_oref);
        object_name = ts_obj_name(object_oref);
        object_parent_id = ts_obj_parent_id(object_oref);
        object_type = ts_obj_type(object_oref);
    }
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1,
             "%s - '%s', id: %u, type: %u, parent: %u -> %s\nAt %s#%d", assert_msg, object_name,
             (unsigned int)object_id, (unsigned int)object_type, (unsigned int)object_parent_id,
             &test_log_buf[0], __FILE__, __LINE__);

    UNITY_TEST_ASSERT_EQUAL_INT(expected_ret, ret, assert_line, &tc_desc[0]);
    if (expected_str != NULL) {
        UNITY_TEST_ASSERT_EQUAL_STRING(expected_str, &test_log_buf[ASSERT_MSG_LOG_SKIP],
                                       assert_line, &tc_desc[0]);
    }
    else {
        UNITY_TEST_ASSERT_EQUAL_UINT16(0, ts_msg_len(response), assert_line, &tc_desc[0]);
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
                           unsigned int assert_line, const char *assert_msg)
{
    int ret;
    thingset_oref_t object_oref = ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID));
    ts_obj_id_t object_id;
    char tc_desc[300];
    ts_obj_id_t names_object_ids[object_count];
    struct ts_msg_stat expected_status_pull_request;

    TS_LOGD("%s", assert_msg + sizeof("Assertion triggered by"));

    expected_status_pull_request.type = TS_MSG_TYPE_REQUEST;
    expected_status_pull_request.code = expected_status_id_pull_request;

    /* Prepare assert message */
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1, "%s\nAt %s#%d", assert_msg, __FILE__, __LINE__);

    thingset_msg_reset(request);
    UNITY_TEST_ASSERT_EQUAL_INT(0, ts_msg_len(request), assert_line, &tc_desc[0]);
    thingset_msg_reset(response);
    UNITY_TEST_ASSERT_EQUAL_INT(0, ts_msg_len(response), assert_line, &tc_desc[0]);

    /*
     * Create fetch request
     * --------------------
     */

    /* Prepare assert message (again) */
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1,
             "%s - %s\nAt %s#%d", assert_msg, (object_path == NULL) ? "<none>" : object_path,
             __FILE__, __LINE__);

    if (object_path != NULL) {
        ret = ts_obj_by_path(ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID)), object_path,
                             (size_t)strlen(object_path), &object_oref);
        UNITY_TEST_ASSERT_EQUAL_INT(0, ret, assert_line, &tc_desc[0]);
    }
    if (test_object_id) {
        UNITY_TEST_ASSERT(ts_obj_db_oref_is_valid(object_oref), assert_line, &tc_desc[0]);
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
            thingset_oref_t fetch_oref;
            ret = ts_obj_by_name(object_oref, name, name_len, &fetch_oref);
            UNITY_TEST_ASSERT_EQUAL_INT(0, ret, assert_line, &tc_desc[0]);
            names_object_ids[j] = ts_obj_id(fetch_oref);
        }
        object_ids = &names_object_ids[0];
        object_names = NULL;
    }
    ret = ts_msg_add_request_fetch_cbor(request, object_id, object_path,
                                        object_count, object_ids, object_names);
    ts_msg_log(request, &test_log_buf[0], sizeof(test_log_buf));

    /* Prepare assert message (again) */
    const char *object_name = "<none>";
    ts_obj_id_t object_parent_id = 0;
    uint8_t object_type = 0;
    if (ts_obj_db_oref_is_object(object_oref)) {
        object_id = ts_obj_id(object_oref);
        object_name = ts_obj_name(object_oref);
        object_parent_id = ts_obj_parent_id(object_oref);
        object_type = ts_obj_type(object_oref);
    }
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1,
        "%s - '%s', id: %u, type: %u, parent: %u -> %s\nAt %s#%d", assert_msg, object_name,
        (unsigned int)object_id, (unsigned int)object_type, (unsigned int)object_parent_id,
        &test_log_buf[0], __FILE__, __LINE__);

    UNITY_TEST_ASSERT_EQUAL_INT(expected_ret_add_request, ret, assert_line, &tc_desc[0]);
    if (expected_str_add_request != NULL) {
        UNITY_TEST_ASSERT_EQUAL_STRING(expected_str_add_request,
                                       &test_log_buf[ASSERT_MSG_LOG_SKIP], assert_line, &tc_desc[0]);
    }
    else {
        UNITY_TEST_ASSERT_EQUAL_UINT16(0, ts_msg_len(request), assert_line, &tc_desc[0]);
    }

    /*
     * Pull fetch request
     * ------------------
     */

    thingset_oref_t request_oref = { .db_id = TEST_INSTANCE_LOCID };
    ts_msg_auth_set(request, thingset_authorisation(TEST_INSTANCE_LOCID));
    ret = ts_msg_pull_request_cbor(request, &request_oref);

    /* Prepare assert message (again) */
    object_name = "<none>";
    if (ts_obj_db_oref_is_object(request_oref)) {
        object_id = ts_obj_id(request_oref);
        object_name = ts_obj_name(request_oref);
        object_parent_id = ts_obj_parent_id(request_oref);
        object_type = ts_obj_type(request_oref);
    }
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1,
        "%s - '%s', id: %u, type: %u, parent: %u, status: 0x%02x\nAt %s#%d",
        assert_msg, object_name, (unsigned int)object_id, (unsigned int)object_type,
        (unsigned int)object_parent_id, (unsigned int)ts_msg_status_code(request),
        __FILE__, __LINE__);

    UNITY_TEST_ASSERT_EQUAL_INT(expected_ret_pull_request, ret, assert_line, &tc_desc[0]);
    UNITY_TEST_ASSERT_EQUAL_UINT8(expected_status_pull_request.type, ts_msg_status_type(request),
                                  assert_line, &tc_desc[0]);
    UNITY_TEST_ASSERT_EQUAL_UINT8(expected_status_pull_request.code, ts_msg_status_code(request),
                                  assert_line, &tc_desc[0]);
    if (test_object_id) {
        UNITY_TEST_ASSERT_EQUAL_UINT16(object_id, ts_obj_id(object_oref), assert_line, &tc_desc[0]);
    }
    UNITY_TEST_ASSERT_EQUAL_UINT16(thingset_authorisation(TEST_INSTANCE_LOCID),
                                   ts_msg_auth(request), assert_line, &tc_desc[0]);

    /*
     * Create fetch response
     * ---------------------
     */

    ret = ts_msg_add_response_fetch_cbor(response, request_oref, request);
    ts_msg_log(response, &test_log_buf[0], sizeof(test_log_buf));

    /* Prepare assert message (again) */
    object_name = "<none>";
    if (ts_obj_db_oref_is_object(request_oref)) {
        object_id = ts_obj_id(request_oref);
        object_name = ts_obj_name(request_oref);
        object_parent_id = ts_obj_parent_id(request_oref);
        object_type = ts_obj_type(request_oref);
    }
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1,
        "%s - '%s', id: %u, type: %u, parent: %u, status: %u -> %s\nAt %s%d",
        assert_msg, object_name, (unsigned int)object_id, (unsigned int)object_type,
        (unsigned int)object_parent_id, (unsigned int)ts_msg_status_code(request),
        &test_log_buf[0], __FILE__, __LINE__);

    UNITY_TEST_ASSERT_EQUAL_INT(expected_ret_add_response, ret, assert_line, &tc_desc[0]);
    if (expected_str_add_response != NULL) {
        UNITY_TEST_ASSERT_EQUAL_STRING(expected_str_add_response,
                                       &test_log_buf[ASSERT_MSG_LOG_SKIP],
                                       assert_line, &tc_desc[0]);
    }
    else {
        UNITY_TEST_ASSERT_EQUAL_UINT16(0, ts_msg_len(response), assert_line, &tc_desc[0]);
    }
}

void assert_msg_add_response_get_json(struct thingset_msg *response, struct thingset_msg *request,
                                      ts_obj_id_t obj_id, int expected_ret,
                                      const char *expected_str,
                                      unsigned int assert_line, const char *assert_msg)
{
    int ret;
    thingset_oref_t object_oref;
    char tc_desc[300];

    /* Prepare assert message */
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1, "%s\nAt %s#%d", assert_msg, __FILE__, __LINE__);

    TS_LOGD("%s", &tc_desc[sizeof("Assertion triggered by")]);

    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    thingset_msg_reset(response);
    UNITY_TEST_ASSERT_EQUAL_UINT16(0, ts_msg_len(response), assert_line, &tc_desc[0]);

    ret = ts_obj_db_oref_by_id(TEST_INSTANCE_LOCID, obj_id, &object_oref );
    UNITY_TEST_ASSERT_EQUAL_INT(0, ret, assert_line, &tc_desc[0]);

    ret = ts_msg_add_response_get_json(response, object_oref, request);

    /* Prepare assert message (again) */
    ts_msg_log(response, &test_log_buf[0], sizeof(test_log_buf));
    snprintf(&tc_desc[0], sizeof(tc_desc) - 1,
             "%s -> ret: %d, oref: %u, name: '%s', >%s<\nAt %s%d", assert_msg,
             ret, (unsigned int)object_oref.db_oid, ts_obj_name(object_oref),
             &test_log_buf[ASSERT_MSG_LOG_SKIP], __FILE__, __LINE__);

    UNITY_TEST_ASSERT_EQUAL_INT(expected_ret, ret, assert_line, &tc_desc[0]);
    UNITY_TEST_ASSERT_EQUAL_STRING_LEN(expected_str, ts_msg_data(response), ts_msg_len(response),
                                       assert_line, &tc_desc[0]);
    UNITY_TEST_ASSERT_EQUAL_UINT16(strlen(expected_str), ts_msg_len(response), assert_line,
                                   &tc_desc[0]);
}
