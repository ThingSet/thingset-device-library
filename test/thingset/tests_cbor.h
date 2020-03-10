
#include "thingset.h"
#include "test_data.h"
#include "unity.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t req_buf[];
extern uint8_t resp_buf[];
extern ThingSet ts;

void cbor_write_array()
{
    char cbor_req_hex[] =
        #if TS_64BIT_TYPES_SUPPORT
        "02 A9 "      // write map with 9 elements
        "19 60 01 01 "                  // value 1
        "19 60 02 02 "
        #else
        "02 A7 "      // write map with 7 elements
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
    TEST_ASSERT_EQUAL_UINT8(TS_STATUS_SUCCESS, resp_buf[0] - 0x80);
}

void cbor_read_array()
{
    char cbor_req_hex[] =
        #if TS_64BIT_TYPES_SUPPORT
        "01 89 "      // read array with 9 elements
        "19 60 01 "
        "19 60 02 "
        #else
        "01 87 "      // read array with 7 elements
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
        "80 89 "     // successful response: array with 9 elements
        "01 "                  // value 1
        "02 "
        #else
        "80 87 "     // successful response: array with 7 elements
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

void cbor_write_int32_array()
{
    // Writing int32 array
    int32_array.type = TS_T_INT32;
    int32_array.num_elements = 4;
    int32_array.max_elements = 100; // Maximum length of the array

    char cbor_req_hex[] =
        "02 A1 "    // write map
        "19 70 03 84 04 02 08 04 ";        // Array [4, 2, 8, 4]

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    memcpy(req_buf, cbor_req, pos);
    ts.process(req_buf, pos, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL_UINT8(TS_STATUS_SUCCESS, resp_buf[0] - 0x80);
}

void cbor_read_int32_array()
{
    // Request: Read int32 array
    char cbor_req_hex[] =
        "01 81 "      // read array
        "19 70 03 ";

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    memcpy(req_buf, cbor_req, pos);
    ts.process(req_buf, pos, resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        "80 "     // successful response:
        "84 04 02 08 04 ";

    uint8_t cbor_resp[100];
    len = strlen(cbor_resp_hex);
    pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_write_float_array()
{
    // Writing float array
    float32_array.type = TS_T_FLOAT32; // Set the array type
    float32_array.num_elements = 2; // Length of the array
    float32_array.max_elements = 100; // Maximum length of the array

    // {28676: [2.27, 3.44]}
    char cbor_req_hex_float[] =
        "02 A1 "    // write map
        "19 70 04 82 FA 40 11 15 40 FA 40 5C 71 E1 "; // Array

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex_float);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex_float[i], NULL, 16);
    }

    memcpy(req_buf, cbor_req, pos);
    ts.process(req_buf, pos, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL_UINT8(TS_STATUS_SUCCESS, resp_buf[0] - 0x80);
}

void cbor_read_float_array()
{
    // Read int32 array
    char cbor_req_hex[] =
        "01 81 "      // read array
        "19 70 04 ";

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    memcpy(req_buf, cbor_req, pos);
    ts.process(req_buf, pos, resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        "80 "     // successful response:
        "82 FA 40 11 15 40 FA 40 5C 71 E1 ";

    uint8_t cbor_resp[100];
    len = strlen(cbor_resp_hex);
    pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_read_rounded()
{
    char cbor_req_hex[] = "01 19 60 0A ";

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    memcpy(req_buf, cbor_req, pos);
    ts.process(req_buf, pos, resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] = "80 08 ";

    uint8_t cbor_resp[100];
    len = strlen(cbor_resp_hex);
    pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_write_rounded()
{
    float tmp = f32;
    char cbor_req_hex[] = "02 A1 19 60 0A 05 ";

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    memcpy(req_buf, cbor_req, pos);
    ts.process(req_buf, pos, resp_buf, TS_RESP_BUFFER_LEN);

    TEST_ASSERT_EQUAL_UINT8(TS_STATUS_SUCCESS, resp_buf[0] - 0x80);
    TEST_ASSERT_EQUAL_FLOAT(5.0, f32);
    f32 = tmp;
}

void cbor_list_ids_input()
{
    // generate list request
    req_buf[0] = TS_INPUT;
    req_buf[1] = 0xF6;     // nullbyte to get response as numeric IDs
    ts.process(req_buf, 2, resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        "80 82 "     // successful response: array with 2 elements
        "19 30 01 "
        "19 30 02 ";

    uint8_t cbor_resp[100];
    int len = strlen(cbor_resp_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_list_names_input()
{
    // generate list request
    req_buf[0] = TS_INPUT;
    req_buf[1] = 0x80;     // empty array to get response as names
    ts.process(req_buf, 2, resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        "80 82 "     // successful response: array with 2 elements
        "6C 6C 6F 61 64 45 6E 54 61 72 67 65 74 "
        "6B 75 73 62 45 6E 54 61 72 67 65 74 ";

    uint8_t cbor_resp[100];
    int len = strlen(cbor_resp_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp_buf, pos);
}

void cbor_list_names_values_input()
{
    // generate list request
    req_buf[0] = TS_INPUT;
    req_buf[1] = 0xA0;     // empty map to get response as names + values
    ts.process(req_buf, 2, resp_buf, TS_RESP_BUFFER_LEN);

    char cbor_resp_hex[] =
        "80 A2 "     // successful response: map with 2 elements
        "6C 6C 6F 61 64 45 6E 54 61 72 67 65 74 "
        "F4 "   // false
        "6B 75 73 62 45 6E 54 61 72 67 65 74 "
        "F4 ";   // false

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
    ts.pub_msg_cbor(resp_buf, TS_RESP_BUFFER_LEN, 0);

    TEST_ASSERT_EQUAL_UINT8(TS_PUBMSG, resp_buf[0]);

    char cbor_resp_hex[] =
        #if TS_64BIT_TYPES_SUPPORT
        "1f A9 "     // map with 9 elements
        "19 60 01 01 "                  // value 1
        "19 60 02 02 "
        #else
        "1f A7 "     // map with 7 elements
        #endif
        "19 60 03 03 "
        "19 60 04 04 "
        "19 60 05 05 "
        "19 60 06 06 "
        "19 60 07 fa 40 fc 7a e1 "      // float32 7.89
        "19 60 08 f5 "                  // true
        "19 60 09 64 74 65 73 74 ";     // string "test"

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
    req_buf[2] = 0x50;     // data object ID 0x5001
    req_buf[3] = 0x01;

    ts.process(req_buf, 4, resp_buf, TS_RESP_BUFFER_LEN);

    TEST_ASSERT_EQUAL(0, resp_buf[0] - 0x80);
    TEST_ASSERT_EQUAL(1, dummy_called_flag);
}

#include "cbor.h"

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


/*
void cbor_get_data_object_name()
{
    char req[3] = { TS_OBJ_NAME, 0x01, 0x00 };   // Obj ID 1
    char resp[100];
    int len = thingset_request(req, 3, resp, 100);

    TEST_ASSERT_EQUAL(TS_OBJ_NAME + 128, resp[0]);
    TEST_ASSERT_EQUAL(T_STRING, resp[1]);
    TEST_ASSERT_EQUAL(sizeof("vSolar")-1, len - 2);     // protocol without nullbyte
    TEST_ASSERT_EQUAL_STRING_LEN("vSolar", &(resp[2]), len - 2);
}

void test_list()
{
    uint8_t req[2] = { TS_LIST, TS_C_CAL };        // category: calibration settings
    uint8_t resp[100];
    int len = thingset_request(req, 2, resp, 100);

    TEST_ASSERT_EQUAL(TS_LIST + 128, resp[0]);
    TEST_ASSERT_EQUAL(TS_T_UINT16, resp[1]);
    TEST_ASSERT_EQUAL(2 + 6*2, len);

    int i = 2;
    TEST_ASSERT_EQUAL_UINT16(100, (resp[i++] + ((uint16_t)resp[i++] << 8)));
    TEST_ASSERT_EQUAL_UINT16(101, (resp[i++] + ((uint16_t)resp[i++] << 8)));
    TEST_ASSERT_EQUAL_UINT16(102, (resp[i++] + ((uint16_t)resp[i++] << 8)));
    TEST_ASSERT_EQUAL_UINT16(103, (resp[i++] + ((uint16_t)resp[i++] << 8)));
    TEST_ASSERT_EQUAL_UINT16(104, (resp[i++] + ((uint16_t)resp[i++] << 8)));
    TEST_ASSERT_EQUAL_UINT16(105, (resp[i++] + ((uint16_t)resp[i++] << 8)));
}

void test_list_small_buffer()
{
    uint8_t req[2] = { TS_LIST, 0 };        // category 0
    uint8_t resp[10];
    int len = req.data.str(req, 2, resp, 10);

    TEST_ASSERT_EQUAL(2, len);
    TEST_ASSERT_EQUAL(TS_LIST + 128, resp[0]);
    TEST_ASSERT_EQUAL(TS_STATUS_RESPONSE_TOO_LONG, resp[1]);
}
*/
