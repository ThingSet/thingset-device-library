/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#include "tests.h"
#include "unity.h"

#include "thingset.h"
#include "cbor.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t req_buf[];
extern uint8_t resp_buf[];
extern ThingSet ts;

extern float f32;
extern int32_t i32;
extern ArrayInfo int32_array;
extern ArrayInfo float32_array;

extern ArrayInfo pub_serial_array;

void cbor_patch_multiple_nodes()
{
    char cbor_req_hex[] =
        "07 02 "
        #if TS_64BIT_TYPES_SUPPORT
        "A9 "      // write map with 9 elements
        "19 60 01 01 "                  // value 1
        "19 60 02 02 "
        #else
        "A7 "      // write map with 7 elements
        #endif
        "19 60 03 03 "
        "19 60 04 04 "
        "19 60 05 05 "
        "19 60 06 06 "
        "19 60 07 fa 40 fc 7a e1 "      // float32 7.89
        "19 60 08 f5 "                  // true
        "19 60 09 64 74 65 73 74 ";        // string "test"

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    memcpy(req_buf, cbor_req, pos);
    ts.process(req_buf, pos, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL_HEX8(TS_STATUS_CHANGED, resp_buf[0]);
}

void cbor_fetch_multiple_nodes()
{
    f32 = 7.89;

    char cbor_req_hex[] =
        "05 18 70 "
        #if TS_64BIT_TYPES_SUPPORT
        "89 "      // read array with 9 elements
        "19 60 01 "
        "19 60 02 "
        #else
        "87 "      // read array with 7 elements
        #endif
        "19 60 03 "
        "19 60 04 "
        "19 60 05 "
        "19 60 06 "
        "19 60 07 "
        "19 60 08 "
        "19 60 09 ";

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    memcpy(req_buf, cbor_req, pos);
    ts.process(req_buf, pos, resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        #if TS_64BIT_TYPES_SUPPORT
        "85 89 "     // successful response: array with 9 elements
        "01 "                  // value 1
        "02 "
        #else
        "85 87 "     // successful response: array with 7 elements
        #endif
        "03 "
        "04 "
        "05 "
        "06 "
        "fa 40 fc 7a e1 "      // float32 7.89
        "f5 "                  // true
        "64 74 65 73 74 ";        // string "test"

    uint8_t cbor_resp[100];
    len = strlen(cbor_resp_hex);
    pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_patch_float_array()
{
    float *arr = (float *)float32_array.ptr;
    arr[0] = 0;
    arr[1] = 0;

    uint8_t req[] = {
        TS_PATCH,
        0x18, TS_CONF,
        0xA1,
            0x19, 0x70, 0x04,
            0x82,
                0xFA, 0x40, 0x11, 0x47, 0xAE,       // 2.27
                0xFA, 0x40, 0x5C, 0x28, 0xF6        // 3.44
    };

    ts.process(req, sizeof(req), resp_buf, TS_RESP_BUFFER_LEN);

    TEST_ASSERT_EQUAL_HEX8(TS_STATUS_CHANGED, resp_buf[0]);
    TEST_ASSERT_EQUAL_FLOAT(2.27, arr[0]);
    TEST_ASSERT_EQUAL_FLOAT(3.44, arr[1]);
}

void cbor_fetch_float_array()
{
    float *arr = (float *)float32_array.ptr;
    arr[0] = 2.27;
    arr[1] = 3.44;

    uint8_t req[] = {
        TS_FETCH,
        0x18, TS_CONF,
        0x19, 0x70, 0x04
    };

    uint8_t resp_expected[] = {
        TS_STATUS_CONTENT,
        0x82,
        0xFA, 0x40, 0x11, 0x47, 0xAE,
        0xFA, 0x40, 0x5C, 0x28, 0xF6
    };

    ts.process(req, sizeof(req), resp_buf, TS_RESP_BUFFER_LEN);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(resp_expected, resp_buf, sizeof(resp_expected));
}

void cbor_fetch_rounded_float()
{
    f32 = 8.4;

    uint8_t req[] = {
        TS_FETCH,
        0x18, TS_CONF,
        0x19, 0x60, 0x0A
    };

    uint8_t resp_expected[] = {
        TS_STATUS_CONTENT,
        0x08
    };

    uint8_t resp[100];
    ts.process(req, sizeof(req), resp, sizeof(resp));

    TEST_ASSERT_EQUAL_HEX8_ARRAY(resp_expected, resp, sizeof(resp_expected));
}

void cbor_patch_rounded_float()
{
    f32 = 0;

    uint8_t req[] = {
        TS_PATCH,
        0x18, TS_CONF,
        0xA1,
            0x19, 0x60, 0x0A,
            0x05
    };

    uint8_t resp[1];
    ts.process(req, sizeof(req), resp, sizeof(resp));

    TEST_ASSERT_EQUAL_HEX8(TS_STATUS_CHANGED, resp[0]);
    TEST_ASSERT_EQUAL_FLOAT(5.0, f32);
}

void cbor_get_output_ids()
{
    uint8_t req[] = { TS_GET, 0x18, TS_OUTPUT, 0xF7 };
    ts.process(req, sizeof(req), resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        "85 83 "     // successful response: array with 3 elements
        "18 71 "
        "18 72 "
        "18 73 ";

    uint8_t cbor_resp[100];
    int len = strlen(cbor_resp_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_get_output_names()
{
    uint8_t req[] = { TS_GET, 0x18, TS_OUTPUT, 0x80 };
    ts.process(req, sizeof(req), resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        "85 83 "     // successful response: array with 3 elements
        "65 42 61 74 5F 56 "
        "65 42 61 74 5F 41 "
        "6C 41 6D 62 69 65 6E 74 5F 64 65 67 43";

    uint8_t cbor_resp[100];
    int len = strlen(cbor_resp_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_get_names_values_output()
{
    uint8_t req[] = { TS_GET, 0x18, TS_OUTPUT, 0xA0 };
    // generate list request
    req_buf[0] = TS_OUTPUT;
    req_buf[1] = 0xA0;     // empty map to get response as names + values
    ts.process(req, sizeof(req), resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        "85 A3 "     // successful response: map with 3 elements
        "65 42 61 74 5F 56 "
        "FA 41 61 99 9A "        // 14.1
        "65 42 61 74 5F 41 "
        "FA 40 A4 28 F6 "        // 5.13
        "6C 41 6D 62 69 65 6E 74 5F 64 65 67 43 "
        "16";

    uint8_t cbor_resp[100];
    int len = strlen(cbor_resp_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_pub_msg()
{
    ts.pub_msg_cbor(resp_buf, TS_RESP_BUFFER_LEN,
        (node_id_t *)pub_serial_array.ptr, pub_serial_array.num_elements);

    TEST_ASSERT_EQUAL_UINT8(TS_PUBMSG, resp_buf[0]);

    char cbor_resp_hex[] =
        "1F A4 "     // map with 4 elements
        "18 1A 1A 00 BC 61 4E "     // int 12345678
        "18 71 FA 41 61 99 9a "     // float 14.10
        "18 72 FA 40 a4 28 f6 "     // float 5.13
        "18 73 16 ";                // int 22

    uint8_t cbor_resp[100];
    int len = strlen(cbor_resp_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

extern bool dummy_called_flag;

void cbor_exec()
{
    dummy_called_flag = 0;

    req_buf[0] = TS_EXEC;     // function ID for exec
    req_buf[1] = 0x19;     // uint16 follows
    req_buf[2] = 0x50;     // data node ID 0x5001
    req_buf[3] = 0x01;

    ts.process(req_buf, 4, resp_buf, TS_RESP_BUFFER_LEN);

    TEST_ASSERT_EQUAL_HEX8(TS_STATUS_VALID, resp_buf[0]);
    TEST_ASSERT_EQUAL(1, dummy_called_flag);
}

void cbor_num_elem()
{
    char cbor_req_hex[] = "B9 F0 00 ";

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    uint16_t num_elements;
    cbor_num_elements(cbor_req, &num_elements);
    TEST_ASSERT_EQUAL(0xF000, num_elements);
}

void cbor_serialize_long_string()
{
    char str[300];
    uint8_t buf[302];

    for (unsigned int i = 0; i < sizeof(str); i++) {
        str[i] = 'T';
    }
    str[299] = '\0';

    int len_total = cbor_serialize_string(buf, str, sizeof(buf));

    TEST_ASSERT_EQUAL_UINT(302, len_total);
    TEST_ASSERT_EQUAL_UINT(0x79, buf[0]);
    TEST_ASSERT_EQUAL_UINT(0x01, buf[1]);   // 0x01 << 8 + 0x2C = 299
    TEST_ASSERT_EQUAL_UINT(0x2B, buf[2]);
}

void tests_cbor()
{
    UNITY_BEGIN();

    RUN_TEST(cbor_get_output_ids);
    RUN_TEST(cbor_get_output_names);
    RUN_TEST(cbor_get_names_values_output);

    RUN_TEST(cbor_patch_multiple_nodes);
    RUN_TEST(cbor_patch_float_array);
    RUN_TEST(cbor_patch_rounded_float);     // writes an integer to float

    RUN_TEST(cbor_fetch_multiple_nodes);
    RUN_TEST(cbor_fetch_float_array);
    RUN_TEST(cbor_fetch_rounded_float);

    RUN_TEST(cbor_pub_msg);
    RUN_TEST(cbor_exec);                    // still previous protocol spec
    RUN_TEST(cbor_num_elem);
    RUN_TEST(cbor_serialize_long_string);

    UNITY_END();
}
