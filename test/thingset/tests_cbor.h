
#include "thingset.h"
#include "test_data.h"
#include "unity.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern ts_buffer_t req, resp;

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
        "19 60 07 fa 40 e0 00 00 "      // float32 7.0
        "19 60 08 f5 "                  // true
        "19 60 09 64 74 65 73 74 ";        // string "test"

    uint8_t cbor_req[100];
    int len = strlen(cbor_req_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_req[pos++] = (char)strtoul(&cbor_req_hex[i], NULL, 16);
    }

    memcpy(req.data.bin, cbor_req, pos);
    req.pos = pos;
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL_UINT8(TS_STATUS_SUCCESS, resp.data.str[0] - 0x80);
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

    memcpy(req.data.bin, cbor_req, pos);
    req.pos = pos;
    thingset_process(&req, &resp, &data);

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
        "fa 40 e0 00 00 "      // float32 7.0
        "f5 "                  // true
        "64 74 65 73 74 ";        // string "test"

    uint8_t cbor_resp[100];
    len = strlen(cbor_resp_hex);
    pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp.data.bin, pos);
}

void cbor_pub_msg()
{
    uint16_t list[] = {
        #if TS_64BIT_TYPES_SUPPORT
        0x6001,
        0x6002,
        #endif
        0x6003,
        0x6004,
        0x6005,
        0x6006,
        0x6007,
        0x6008,
        0x6009
    };

    int status = thingset_pub_msg_cbor(&resp, &data, list, sizeof(list)/sizeof(uint16_t));

    TEST_ASSERT_EQUAL(0, status);

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
        "19 60 07 fa 40 e0 00 00 "      // float32 7.0
        "19 60 08 f5 "                  // true
        "19 60 09 64 74 65 73 74 ";     // string "test"

    uint8_t cbor_resp[100];
    int len = strlen(cbor_resp_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_resp[pos++] = (char)strtoul(&cbor_resp_hex[i], NULL, 16);
    }

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_resp, resp.data.bin, pos);
}

extern bool dummy_called_flag;

void cbor_exec()
{
    dummy_called_flag = 0;
    
    req.data.bin[0] = 0x07;     // function ID for exec
    req.data.bin[1] = 0x19;     // uint16 follows
    req.data.bin[2] = 0x50;     // data object ID 0x5001
    req.data.bin[3] = 0x01;
    req.pos = 4;
    thingset_process(&req, &resp, &data);

    TEST_ASSERT_EQUAL(0, resp.data.bin[0] - 0x80);
    TEST_ASSERT_EQUAL(1, dummy_called_flag);
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