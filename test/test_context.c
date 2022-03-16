/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>

#include "test.h"

struct ts_context ts;
uint8_t req_buf[TS_REQ_BUFFER_LEN];
uint8_t resp_buf[TS_RESP_BUFFER_LEN];

bool group_callback_called;
bool update_callback_called;
bool dummy_called_flag;
struct ts_array_info pub_serial_array;


void dummy(void)
{
    dummy_called_flag = true;
}

void group_callback(void)
{
    group_callback_called = true;
}

void update_callback(void)
{
    update_callback_called = true;
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
        ts_set_authentication(&ts, TS_EXP_MASK | TS_USR_MASK);
    }
    else if (strlen(pass_mkr) == strlen(auth_password) &&
        strncmp(auth_password, pass_mkr, strlen(pass_mkr)) == 0)
    {
        ts_set_authentication(&ts, TS_MKR_MASK | TS_USR_MASK);
    }
    else {
        ts_set_authentication(&ts, TS_USR_MASK);
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
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(exp_b, resp_buf, resp_len, msg);
}

void assert_bin_req(const uint8_t *req_b, int req_len, const char *exp_hex, const char* msg)
{
    int resp_len = ts_process(&ts, req_b, req_len, resp_buf, TS_RESP_BUFFER_LEN);

    assert_bin_resp(resp_buf, resp_len, exp_hex, msg);
}

void assert_bin_req_exp_bin(const uint8_t *req_b, int req_len, const uint8_t *exp_b, int exp_len, const char* msg)
{
    char exp_hex[exp_len * 3 + 1];
    (void)_bin2hex(exp_hex, sizeof(exp_hex), exp_b, exp_len);

    assert_bin_req(req_b, req_len, exp_hex, msg);
}

void assert_bin_req_hex(const char *req_hex, const char *exp_hex, const char *msg)
{
    int req_len = _hex2bin(req_buf, TS_REQ_BUFFER_LEN, req_hex);

    TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(TS_REQ_BUFFER_LEN, req_len, msg);

    assert_bin_req(req_buf, req_len, exp_hex, msg);
}

void assert_txt_resp(int exp_len, const char *exp_s, const char *msg)
{
    int resp_buf_len = strlen((char *)resp_buf);

    TEST_ASSERT_EQUAL_MESSAGE(exp_len, resp_buf_len, msg);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(exp_s, (char *)resp_buf, msg);
}

void assert_txt_req(const char *req_s, const char *exp_s, const char *msg)
{
    int req_len = strlen(req_s);
    TEST_ASSERT_LESS_OR_EQUAL_size_t_MESSAGE(TS_REQ_BUFFER_LEN - 1, req_len, msg);

    strncpy((char *)req_buf, req_s, TS_REQ_BUFFER_LEN);
    int resp_len = ts_process(&ts, req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    assert_txt_resp(resp_len, exp_s, msg);
}

static void _txt_patch(char const *name, char const *value, const char *msg)
{
    int req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "=conf {\"%s\":%s}", name, value);
    int resp_len = ts_process(&ts, req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    int resp_buf_len = strlen((char *)resp_buf);

    TEST_ASSERT_EQUAL_MESSAGE(resp_len, resp_buf_len, msg);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(":84 Changed.", (char *)resp_buf, msg);
}

static int _txt_fetch(char const *name, char *value_read, const char *msg)
{
    int req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "?conf \"%s\"", name);
    int resp_len = ts_process(&ts, req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    int resp_buf_len = strlen((char *)resp_buf);

    TEST_ASSERT_EQUAL_MESSAGE(resp_len, resp_buf_len, msg);

    int pos_dot = strchr((char *)resp_buf, '.') - (char *)resp_buf + 1;
    char buf[100];
    strncpy(buf, (char *)resp_buf, pos_dot);
    buf[pos_dot] = '\0';

    TEST_ASSERT_EQUAL_STRING_MESSAGE(":85 Content.", buf, msg);

    return snprintf(value_read, strlen((char *)resp_buf) - pos_dot, "%s", resp_buf + pos_dot + 1);
}

// returns length of read value
static int _bin_fetch(uint16_t id, char *value_read, const char *msg)
{
    uint8_t req[] = {
        TS_FETCH,
        0x18, ID_CONF,
        0x19, (uint8_t)(id >> 8), (uint8_t)id
    };
    ts_process(&ts, req, sizeof(req), resp_buf, TS_RESP_BUFFER_LEN);

    TEST_ASSERT_EQUAL_HEX8_MESSAGE(TS_STATUS_CONTENT, resp_buf[0], msg);

    int value_len = cbor_size((uint8_t*)resp_buf + 1);
    memcpy(value_read, resp_buf + 1, value_len);
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
    unsigned int len = cbor_size((uint8_t*)value);
    TEST_ASSERT_LESS_THAN_size_t_MESSAGE(sizeof(req) - 7, len, msg);

    memcpy(req + 7, value, len);
    ts_process(&ts, req, len + 7, resp_buf, TS_RESP_BUFFER_LEN);

    TEST_ASSERT_EQUAL_HEX8_MESSAGE(TS_STATUS_CHANGED, resp_buf[0], msg);
}

void assert_json2cbor(char const *name, char const *json_value, uint16_t id, const char *const cbor_value_hex, const char *msg)
{
    char buf[100];  // temporary data storage (JSON or CBOR)
    uint8_t cbor_value[100];
    int len = _hex2bin(cbor_value, sizeof(cbor_value), (char *)cbor_value_hex);

    _txt_patch(name, json_value, msg);
    len = _bin_fetch(id, buf, msg);

    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(cbor_value, buf, len, msg);
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
