/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

void test_bin_get_meas_ids_values(void)
{
    const uint8_t req[] = { TS_GET, ID_MEAS };
    const char resp_hex[] =
        "85 A3 "     // successful response: map with 3 elements
        "18 71 "
        "FA 41 61 99 9A "        // 14.1
        "18 72 "
        "FA 40 A4 28 F6 "        // 5.13
        "18 73 "
        "16";

    TEST_ASSERT_BIN_REQ(req, sizeof(req), resp_hex);
}

void test_bin_get_meas_names_values()
{
    const uint8_t req[] = { TS_GET, 0x64, 0x6D, 0x65, 0x61, 0x73 };
    const char resp_hex[] =
        "85 A3 "     // successful response: map with 3 elements
        "65 42 61 74 5F 56 "
        "FA 41 61 99 9A "        // 14.1
        "65 42 61 74 5F 41 "
        "FA 40 A4 28 F6 "        // 5.13
        "6C 41 6D 62 69 65 6E 74 5F 64 65 67 43 "
        "16";

    TEST_ASSERT_BIN_REQ(req, sizeof(req), resp_hex);
}

void test_bin_get_single_value()
{
    const uint8_t req[] = { TS_GET, 0x18, 0x71 };
    const char resp_hex[] = "85 FA 41 61 99 9A "; // 14.1

    TEST_ASSERT_BIN_REQ(req, sizeof(req), resp_hex);
}

void test_bin_fetch_meas_ids(void)
{
    const uint8_t req[] = { TS_FETCH, ID_MEAS, 0xF7 };
    const char resp_hex[] =
        "85 83 "     // successful response: array with 3 elements
        "18 71 "
        "18 72 "
        "18 73 ";

    TEST_ASSERT_BIN_REQ(req, sizeof(req), resp_hex);
}

void test_bin_fetch_meas_names(void)
{
    const uint8_t req[] = { TS_FETCH,
        0x64, 0x6D, 0x65, 0x61, 0x73,   // "meas"
        0xF7 };                         // CBOR undefined
    const char resp_hex[] =
        "85 83 "     // successful response: array with 3 elements
        "65 42 61 74 5F 56 "
        "65 42 61 74 5F 41 "
        "6C 41 6D 62 69 65 6E 74 5F 64 65 67 43";

    TEST_ASSERT_BIN_REQ(req, sizeof(req), resp_hex);
}

void test_bin_patch_multiple_objects(void)
{
    const char req_hex[] =
        "07 06 "
#if TS_CONFIG_64BIT_TYPES_SUPPORT
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
    const char resp_hex[] = "84 "; // TS_STATUS_CHANGED

    TEST_ASSERT_BIN_REQ_HEX(req_hex, resp_hex);
}

void test_bin_fetch_multiple_objects(void)
{
    f32 = 7.89;

    char req_hex[] =
        "05 06 "
#if TS_CONFIG_64BIT_TYPES_SUPPORT
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
    const char resp_hex[] =
#if TS_CONFIG_64BIT_TYPES_SUPPORT
        "85 89 "     // successful response: array with 9 elements
        "01 "        // value 1
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
        "64 74 65 73 74 ";

    TEST_ASSERT_BIN_REQ_HEX(req_hex, resp_hex);
}

void test_bin_patch_float_array(void)
{
    float *arr = (float *)float32_array.ptr;
    arr[0] = 0;
    arr[1] = 0;

    const uint8_t req[] = {
        TS_PATCH,
        0x18, ID_CONF,
        0xA1,
            0x19, 0x70, 0x04,
            0x82,
                0xFA, 0x40, 0x11, 0x47, 0xAE,       // 2.27
                0xFA, 0x40, 0x5C, 0x28, 0xF6        // 3.44
    };
    const uint8_t resp_expected[] = {
        TS_STATUS_CHANGED };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));

    TEST_ASSERT_EQUAL_FLOAT(2.27, arr[0]);
    TEST_ASSERT_EQUAL_FLOAT(3.44, arr[1]);
}

void test_bin_fetch_float_array(void)
{
    float *arr = (float *)float32_array.ptr;
    arr[0] = 2.27;
    arr[1] = 3.44;

    const uint8_t req[] = {
        TS_FETCH,
        0x18, ID_CONF,
        0x19, 0x70, 0x04
    };
    const uint8_t resp_expected[] = {
        TS_STATUS_CONTENT,
        0x82,
        0xFA, 0x40, 0x11, 0x47, 0xAE,
        0xFA, 0x40, 0x5C, 0x28, 0xF6
    };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));
}

void test_bin_fetch_rounded_float(void)
{
    f32 = 8.4;

    const uint8_t req[] = {
        TS_FETCH,
        0x18, ID_CONF,
        0x19, 0x60, 0x0A
    };
    const uint8_t resp_expected[] = {
        TS_STATUS_CONTENT,
        0x08
    };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));
}

void test_bin_patch_rounded_float(void)
{
    f32 = 0;

    const uint8_t req[] = {
        TS_PATCH,
        0x18, ID_CONF,
        0xA1,
            0x19, 0x60, 0x0A,
            0x05
    };
    const uint8_t resp_expected[] = {
        TS_STATUS_CHANGED };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));

    TEST_ASSERT_EQUAL_FLOAT(5.0, f32);
}

void test_bin_statement_subset(void)
{
    const char resp_expected[] =
        "1F "
        "0A "                   // ID of "report"
        "84 "                   // array with 4 elements
        "1A 00 BC 61 4E "       // int 12345678
        "FA 41 61 99 9a "       // float 14.10
        "FA 40 a4 28 f6 "       // float 5.13
        "16 ";                  // int 22

    int resp_len = thingset_bin_statement_by_path(test_resp_buf, sizeof(test_resp_buf), "report");

    TEST_ASSERT_BIN_RESP(test_resp_buf, resp_len, resp_expected);

    resp_len = thingset_bin_statement_by_id(test_resp_buf, sizeof(test_resp_buf), ID_REPORT);

    TEST_ASSERT_BIN_RESP(test_resp_buf, resp_len, resp_expected);
}

void test_bin_statement_group(void)
{
    const char resp_expected[] =
        "1F "                                       // TS_MSG_CODE_STATEMENT
        "01 "                                       // ID of "info"
        "83 "                                       // array with 3 elements
        "6B 4C 69 62 72 65 20 53 6F 6C 61 72 "      // "Libre Solar"
        "1A 00 BC 61 4E "                           // int 12345678
        "68 41 42 43 44 31 32 33 34 ";              // "ABCD1234"

    int resp_len = thingset_bin_statement_by_path(test_resp_buf, sizeof(test_resp_buf), "info");

    TEST_ASSERT_BIN_RESP(test_resp_buf, resp_len, resp_expected);

    resp_len = thingset_bin_statement_by_id(test_resp_buf, sizeof(test_resp_buf), ID_INFO);

    TEST_ASSERT_BIN_RESP(test_resp_buf, resp_len, resp_expected);
}

void test_bin_import(void)
{
    const char req_hex[] =
        "86 "                       // TS_MSG_CODE_EXPORT
        "A2 "                       // map with 2 elements
        "18 31 FA 41 61 99 9a "     // float 14.10
        "18 32 FA 40 a4 28 f6 ";    // float 5.13
    int req_buf_len = _hex2bin(test_req_buf, sizeof(test_req_buf), req_hex);

    int ret = thingset_bin_import(test_req_buf, req_buf_len, TS_WRITE_MASK, SUBSET_REPORT);

    TEST_ASSERT_EQUAL(TS_STATUS_CHANGED, ret);
}

void test_bin_exec(void)
{
    test_core_dummy_called = 0;

    const uint8_t req[] = {
        TS_POST,
        0x19, 0x50, 0x01,       // object ID as endpoint
        0x80                    // empty array (no parameters)
    };
    const uint8_t resp_expected[] = {
        TS_STATUS_VALID };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));

    TEST_ASSERT_EQUAL(1, test_core_dummy_called);
}

void test_bin_patch_fetch_bytes(void)
{
    const uint8_t req_patch[] = {
        TS_PATCH,
        0x18, ID_CONF,
        0xA1,
            0x19, 0x80, 0x00,
            0x48, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07        // write 8 bytes
    };
    const uint8_t resp_patch_expected[] = {
        TS_STATUS_CHANGED };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req_patch, sizeof(req_patch), resp_patch_expected, sizeof(resp_patch_expected));

    TEST_ASSERT_EQUAL_UINT16(8, bytes_buf.num_bytes);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&req_patch[8], &bytes_buf.bytes[0], 8);

    const uint8_t req_get[] = { TS_FETCH, 0x18, ID_CONF, /*0x81,*/ 0x19, 0x80, 0x00 };
    uint8_t resp_get_expected[] = {
        0x85, /*0x81,*/
        0x48, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
    };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req_get, sizeof(req_get), resp_get_expected, sizeof(resp_get_expected));
}

void test_bin_export(void)
{
    const char resp_expected[] =
        "86 "                       // TS_MSG_CODE_EXPORT
        "A4 "                       // map with 4 elements
        "18 1A 1A 00 BC 61 4E "     // int 12345678
        "18 71 FA 41 61 99 9a "     // float 14.10
        "18 72 FA 40 a4 28 f6 "     // float 5.13
        "18 73 16 ";                // int 22

    int resp_len = thingset_bin_export(test_req_buf, sizeof(test_req_buf), SUBSET_REPORT);

    TEST_ASSERT_BIN_RESP(test_req_buf, resp_len, resp_expected);
}

void tests_bin(void)
{
    UNITY_BEGIN();

    // GET request
    RUN_TEST(test_bin_get_meas_ids_values);
    RUN_TEST(test_bin_get_meas_names_values);
    RUN_TEST(test_bin_get_single_value);

    // PATCH request
    RUN_TEST(test_bin_patch_multiple_objects);
    RUN_TEST(test_bin_patch_float_array);
    RUN_TEST(test_bin_patch_rounded_float);     // writes an integer to float

    // FETCH request
    RUN_TEST(test_bin_fetch_meas_ids);
    RUN_TEST(test_bin_fetch_meas_names);
    RUN_TEST(test_bin_fetch_multiple_objects);
    RUN_TEST(test_bin_fetch_float_array);
    RUN_TEST(test_bin_fetch_rounded_float);

    // POST request
    RUN_TEST(test_bin_exec);

    // pub/sub messages
    RUN_TEST(test_bin_statement_subset);
    RUN_TEST(test_bin_statement_group);
    //RUN_TEST(test_bin_pub_can);

    // general tests
    //RUN_TEST(test_bin_num_elem);
    //RUN_TEST(test_bin_serialize_long_string);

    // binary (bytes) data type
#if TS_BYTE_STRING_TYPE_SUPPORT
    RUN_TEST(test_bin_serialize_bytes);
    RUN_TEST(test_bin_deserialize_bytes);
    RUN_TEST(test_bin_patch_fetch_bytes);
#endif

    // data export/import
    RUN_TEST(test_bin_export);
    RUN_TEST(test_bin_import);

    UNITY_END();
}
