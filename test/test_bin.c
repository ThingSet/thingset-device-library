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
    const uint8_t req[] = { TS_GET, 0x64, 0x4D, 0x65, 0x61, 0x73 };
    const char resp_hex[] =
        "85 A3 "     // successful response: map with 3 elements
        "66 72 42 61 74 5F 56 "
        "FA 41 61 99 9A "        // 14.1
        "66 72 42 61 74 5F 41 "
        "FA 40 A4 28 F6 "        // 5.13
        "6D 72 41 6D 62 69 65 6E 74 5F 64 65 67 43 "
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
        0x64, 0x4D, 0x65, 0x61, 0x73,   // "Meas"
        0xF7 };                         // CBOR undefined
    const char resp_hex[] =
        "85 83 "     // successful response: array with 3 elements
        "66 72 42 61 74 5F 56 "
        "66 72 42 61 74 5F 41 "
        "6D 72 41 6D 62 69 65 6E 74 5F 64 65 67 43";

    TEST_ASSERT_BIN_REQ(req, sizeof(req), resp_hex);
}

void test_bin_patch_multiple_objects(void)
{
    const char req_hex[] =
        "07 06 "
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
    const char resp_hex[] = "84 "; // TS_STATUS_CHANGED

    TEST_ASSERT_BIN_REQ_HEX(req_hex, resp_hex);
}

void test_bin_fetch_multiple_objects(void)
{
    f32 = 7.89;

    char req_hex[] =
        "05 06 "
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
    const char resp_hex[] =
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
        "64 74 65 73 74 ";

    TEST_ASSERT_BIN_REQ_HEX(req_hex, resp_hex);
}

void test_bin_fetch_by_name(void)
{
    char req_hex[] =
        "05 "
        "64 4D 65 61 73 "           // "Meas"
        "82 "                       // array with 2 elements
        "66 72 42 61 74 5F 56 "     // "rBat_V"
        "66 72 42 61 74 5F 41 ";    // "rBat_A"

    const char resp_hex[] =
        "85 82 "                // successful response: array with 2 elements
        "FA 41 61 99 9A "       // 14.1
        "FA 40 A4 28 F6 ";      // 5.13

    TEST_ASSERT_BIN_REQ_HEX(req_hex, resp_hex);
}

void test_bin_patch_float_array(void)
{
    float *arr = (float *)float32_array.elements;
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
    float *arr = (float *)float32_array.elements;
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
        TS_STATUS_CHANGED
    };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));

    TEST_ASSERT_EQUAL_FLOAT(5.0, f32);
}

void test_bin_fetch_num_records()
{
    const uint8_t req[] = {
        TS_FETCH,
        0x19, 0x70, 0x05,
        0xF7 // CBOR undefined
    };
    const uint8_t resp_expected[] = {
        0x85, 0x02
    };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));
}

void test_bin_fetch_record()
{
    const uint8_t req[] = {
        TS_FETCH,
        0x19, 0x70, 0x05,
        0x01 // second record
    };
    const uint8_t resp_expected[] = {
        0x85,
        0xA3,
            0x18, 0x81,
            0x18, 0x7B, // 123
            0x18, 0x82,
            0xFA, 0x41, 0x68, 0x00, 0x00, // 14.5
            0x18, 0x83,
            0x02
    };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));
}

void test_bin_fetch_record_item()
{
    /* a sigle item cannot be fetched from a record, as the IDs are not unique */
    const uint8_t req[] = {
        TS_FETCH,
        0x00,
        0x81, // array with single item
            0x18, 0x81, // data object ID
    };
    const uint8_t resp_expected[] = {
        0xA4 // Not found.
    };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));
}

void test_bin_statement_subset(void)
{
    const char resp_expected[] =
        "1F "
        "0A "                   // ID of "mReport"
        "84 "                   // array with 4 elements
        "1A 00 BC 61 4E "       // int 12345678
        "FA 41 61 99 9a "       // float 14.10
        "FA 40 a4 28 f6 "       // float 5.13
        "16 ";                  // int 22

    int resp_len = ts_bin_statement_by_path(&ts, resp_buf, sizeof(resp_buf), "mReport");

    TEST_ASSERT_BIN_RESP(resp_buf, resp_len, resp_expected);

    resp_len = ts_bin_statement_by_id(&ts, resp_buf, sizeof(resp_buf), ID_REPORT);

    TEST_ASSERT_BIN_RESP(resp_buf, resp_len, resp_expected);
}

void test_bin_statement_group(void)
{
    const char resp_expected[] =
        "1F "
        "01 "                                       // ID of "Info"
        "82 "                                       // array with 2 elements
        "6B 4C 69 62 72 65 20 53 6F 6C 61 72 "      // "Libre Solar"
        "68 41 42 43 44 31 32 33 34 ";              // "ABCD1234"

    int resp_len = ts_bin_statement_by_path(&ts, resp_buf, sizeof(resp_buf), "Info");

    TEST_ASSERT_BIN_RESP(resp_buf, resp_len, resp_expected);

    resp_len = ts_bin_statement_by_id(&ts, resp_buf, sizeof(resp_buf), ID_INFO);

    TEST_ASSERT_BIN_RESP(resp_buf, resp_len, resp_expected);
}

void test_bin_pub_can(void)
{
    int start_pos = 0;
    uint32_t msg_id;
    uint8_t can_data[8];

    const uint8_t rBat_V_hex[] = { 0xFA, 0x41, 0x61, 0x99, 0x9a };
    const uint8_t rBat_A_hex[] = { 0xFA, 0x40, 0xa4, 0x28, 0xf6 };

    // first call (should return rBat_V)
    int can_data_len = ts_bin_pub_can(&ts, &start_pos, SUBSET_CAN, 123, &msg_id, &can_data[0]);
    TEST_ASSERT_NOT_EQUAL(-1, can_data_len);

    uint16_t can_dev_id = (msg_id & 0x00FFFF00) >> 8;
    uint32_t can_subsets = TS_CAN_PUBSUB(msg_id);
    uint32_t can_prio = msg_id & TS_CAN_PRIO_MASK;
    TEST_ASSERT_EQUAL_UINT16(0x71, can_dev_id);
    TEST_ASSERT_EQUAL_UINT32(TS_CAN_PRIO_PUBSUB_LOW, can_prio);
    TEST_ASSERT(can_subsets);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&rBat_V_hex[0], &can_data[0], sizeof(rBat_V_hex));

    // second call (should return rBat_A)
    can_data_len = ts_bin_pub_can(&ts, &start_pos, SUBSET_CAN, 123, &msg_id, &can_data[0]);
    TEST_ASSERT_NOT_EQUAL(-1, can_data_len);

    can_dev_id = (msg_id & 0x00FFFF00) >> 8;
    can_subsets = TS_CAN_PUBSUB(msg_id);
    can_prio = msg_id & TS_CAN_PRIO_MASK;
    TEST_ASSERT_EQUAL_UINT16(0x72, can_dev_id);
    TEST_ASSERT_EQUAL_UINT32(TS_CAN_PRIO_PUBSUB_LOW, can_prio);
    TEST_ASSERT(can_subsets);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&rBat_A_hex[0], &can_data[0], sizeof(rBat_A_hex));

    // third call (should not find further objects)
    can_data_len = ts_bin_pub_can(&ts, &start_pos, SUBSET_CAN, 123, &msg_id, &can_data[0]);
    TEST_ASSERT_EQUAL(-1, can_data_len);
}

void test_bin_import(void)
{
    const char req_hex[] =
        "A2 "     // map with 2 elements
        "18 31 FA 41 61 99 9a "     // float 14.10
        "18 32 FA 40 a4 28 f6 ";    // float 5.13
    int req_buf_len = _hex2bin(req_buf, sizeof(req_buf), req_hex);

    int ret = ts_bin_import(&ts, req_buf, req_buf_len, TS_WRITE_MASK, SUBSET_REPORT);

    TEST_ASSERT_EQUAL(TS_STATUS_CHANGED, ret);
}

void test_bin_exec(void)
{
    dummy_called_flag = 0;

    const uint8_t req[] = {
        TS_POST,
        0x19, 0x50, 0x01,       // object ID as endpoint
        0x80                    // empty array (no parameters)
    };
    const uint8_t resp_expected[] = {
        TS_STATUS_VALID };

    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));

    TEST_ASSERT_EQUAL(1, dummy_called_flag);
}

void test_bin_num_elem(void)
{
    const uint8_t req[] = { 0xB9, 0xF0, 0x00 };
    uint16_t num_elements;

    cbor_num_elements(&req[0], &num_elements);

    TEST_ASSERT_EQUAL_UINT16(0xF000, num_elements);
}

void test_bin_serialize_long_string(void)
{
    /* use public buffers for testing - assure sufficient size */
    char *str = (char *)&req_buf[0];
    uint8_t *buf = &resp_buf[0];
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(301, sizeof(req_buf));
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(302, sizeof(resp_buf));

    for (unsigned int i = 0; i < 301; i++) {
        str[i] = 'T';
    }
    str[256] = '\0';    // terminate string after 256 bytes

    int len_total = cbor_serialize_string(buf, str, 302);

    TEST_ASSERT_EQUAL_UINT(256 + 3, len_total);     // strlen + below 3 bytes
    TEST_ASSERT_EQUAL_UINT(0x79, buf[0]);
    TEST_ASSERT_EQUAL_UINT(0x01, buf[1]);
    TEST_ASSERT_EQUAL_UINT(0x00, buf[2]);           // null-termination is not stored
}

#if TS_BYTE_STRING_TYPE_SUPPORT
void test_bin_serialize_bytes(void)
{
    /* use public buffers for testing - assure sufficient size */
    uint8_t *bytes = &req_buf[0];
    uint8_t *cbor = &resp_buf[0];
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(300, sizeof(req_buf));
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(400, sizeof(resp_buf));

    for (unsigned int i = 0; i < 300; i++) {
        bytes[i] = i % 256;
    }

    // this should consider 0 as normal bytes and not terminate there
    int len_total = cbor_serialize_bytes(cbor, bytes, 300, 400);

    TEST_ASSERT_EQUAL_UINT(303, len_total);
    TEST_ASSERT_EQUAL_UINT(0x59, cbor[0]);
    TEST_ASSERT_EQUAL_UINT(0x01, cbor[1]);   // 0x01 << 8 + 0x2C = 300
    TEST_ASSERT_EQUAL_UINT(0x2C, cbor[2]);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(bytes, &cbor[3], 300);
}

void test_bin_deserialize_bytes(void)
{
    /* use public buffers for testing - assure sufficient size */
    uint8_t *bytes = &req_buf[0];
    uint8_t *cbor = &resp_buf[0];
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(300, sizeof(req_buf));
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(400, sizeof(resp_buf));

    cbor[0] = 0x59;
    cbor[1] = 0x01;
    cbor[2] = 0x2C;
    for (unsigned int i = 0; i < 300; i++) {
        cbor[i + 3] = i % 256;
    }

    uint16_t num_bytes;
    int len_total = cbor_deserialize_bytes(cbor, bytes, 300, &num_bytes);

    TEST_ASSERT_EQUAL_UINT(303, len_total);
    TEST_ASSERT_EQUAL_UINT(300, num_bytes);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&bytes[0], &cbor[3], 300);
}
#endif /* TS_BYTE_STRING_TYPE_SUPPORT */

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
        "A4 "                       // map with 4 elements
        "10 1A 00 BC 61 4E "        // int 12345678
        "18 71 FA 41 61 99 9a "     // float 14.10
        "18 72 FA 40 a4 28 f6 "     // float 5.13
        "18 73 16 ";                // int 22

    int resp_len = ts_bin_export(&ts, resp_buf, sizeof(resp_buf), SUBSET_REPORT);

    TEST_ASSERT_BIN_RESP(resp_buf, resp_len, resp_expected);
}

void test_bin_update_callback(void)
{
    const uint8_t req[] = {
        TS_PATCH,
        0x18, ID_CONF,
        0xA1,
            0x18, 0x31,
            0x05
    };
    const uint8_t resp_expected[] = {
        TS_STATUS_CHANGED
    };

    update_callback_called = false;

    // without callback
    ts_set_update_callback(&ts, SUBSET_NVM, NULL);
    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));
    TEST_ASSERT_EQUAL(false, update_callback_called);

    // with configured callback
    ts_set_update_callback(&ts, SUBSET_NVM, update_callback);
    TEST_ASSERT_BIN_REQ_EXP_BIN(req, sizeof(req), resp_expected, sizeof(resp_expected));
    TEST_ASSERT_EQUAL(true, update_callback_called);
}

void test_bin_fetch_paths(void)
{
    const char req[] =
        "05 "       // FETCH
        "17 "       // _paths
        "84 "       // array with 4 elements
        "18 71 "    // uint 0x71
        "18 81 "    // uint 0x81
        "10 "       // uint 0x10
        "01 ";      // uint 0x01 (ID_INFO)

    const char resp_expected[] =
        "85 "                               // status: content
        "84 "                               // array with 4 elements
        "6B 4D 65 61 73 2F 72 42 61 74 5F 56 " // string "Meas/rBat_V"
        "67 4C 6F 67 2F 74 5F 73 "          // string "Log/t_s"
        "63 74 5F 73 "                      // string "t_s"
        "64 49 6E 66 6F ";                  // string "Info"

    TEST_ASSERT_BIN_REQ_HEX(req, resp_expected);
}

void test_bin_fetch_ids(void)
{
    const char req[] =
        "05 "                               // FETCH
        "16 "                               // _ids
        "84 "                               // array with 4 elements
        "6B 4D 65 61 73 2F 72 42 61 74 5F 56 " // string "Meas/rBat_V"
        "67 4C 6F 67 2F 74 5F 73 "          // string "Log/t_s"
        "63 74 5F 73 "                      // string "t_s"
        "64 49 6E 66 6F ";                  // string "Info"

    const char resp_expected[] =
        "85 "       // status: content
        "84 "       // array with 4 elements
        "18 71 "    // uint 0x71
        "18 81 "    // uint 0x81
        "10 "       // uint 0x10
        "01 ";      // uint 0x01 (ID_INFO)

    TEST_ASSERT_BIN_REQ_HEX(req, resp_expected);
}
