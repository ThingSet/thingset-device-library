/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

#include <math.h>

void test_msg_alloc(void)
{
    int ret;
    struct thingset_msg *msg = NULL;

    /* Check buffer pool size for testing */
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(16,  TS_CONFIG_BUF_COUNT);
    TEST_ASSERT_GREATER_OR_EQUAL_size_t(1024, TS_CONFIG_BUF_DATA_SIZE);

    ret = thingset_msg_alloc(128, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);

    thingset_msg_unref(msg);
}

void test_msg_add_primitives(void)
{
    int ret;
    struct thingset_msg *msg = NULL;
    float       val_f32;
    bool        val_bool;
    uint16_t    val_u16;
    uint32_t    val_u32;
    uint64_t    val_u64;
    const char *val_str;
    int16_t     val_i16;
    int32_t     val_i32;
    int64_t     val_i64;
    CborType    cbor_type;

    ret = thingset_msg_alloc(32, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    /*
     * Values - raw
     * ------------
     */

    ret = ts_msg_add_u8(msg, (uint8_t)'1');
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));

    ret = ts_msg_add_u8(msg, (uint8_t)'2');
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(2, ts_msg_len(msg));

    ret = ts_msg_add_mem(msg, (const uint8_t *)&"3456", 4);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("123456", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(6, ts_msg_len(msg));

    /*
     * Bool - JSON
     * -----------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_bool_json(msg, true);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("true", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(4, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_bool_json(msg, &val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(true, val_bool);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_bool_json(msg, false);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("false", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(5, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_bool_json(msg, &val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(false, val_bool);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u8(msg, (uint8_t)'0');
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_bool_json(msg, &val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(false, val_bool);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_mem(msg, (const uint8_t *)"12345", 5);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(5, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_bool_json(msg, &val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(true, val_bool);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /*
     * DecFrac - JSON
     * --------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_decfrac_json(msg, INT32_MAX, INT16_MAX);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_decfrac_json(msg, &val_i32, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, val_i32);
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, val_i16);

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_decfrac_json(msg, INT32_MIN, INT16_MIN);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_decfrac_json(msg, &val_i32, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, val_i32);
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, val_i16);

    /*
     * Float - JSON
     * ------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_decfrac_json(msg, INT32_MAX, INT16_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("2147483647e32767", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(16, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_decfrac_json(msg, &val_i32, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, val_i32);
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, val_i16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_decfrac_json(msg, INT32_MIN, INT16_MIN);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("-2147483648e-32768", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(18, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_decfrac_json(msg, &val_i32, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, val_i32);
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, val_i16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_f32_json(msg, 123.45678, 5);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("123.45678", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(9, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_f32_json(msg, &val_f32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_FLOAT(123.45678, val_f32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_f32_json(msg, 1.0/0.0, 5);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("null", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(4, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_f32_json(msg, &val_f32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_FLOAT(NAN, val_f32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_f32_json(msg, 0.0/0.0, 5);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("null", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(4, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_f32_json(msg, &val_f32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_FLOAT(NAN, val_f32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /*
     * Signed Integer - JSON
     * ---------------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i16_json(msg, INT16_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("32767", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(5, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i16_json(msg, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, val_i16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i16_json(msg, INT16_MIN);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("-32768", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(6, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i16_json(msg, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, val_i16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i32_json(msg, INT32_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("2147483647", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(10, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i32_json(msg, &val_i32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, val_i32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i32_json(msg, INT32_MIN);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("-2147483648", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(11, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i32_json(msg, &val_i32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, val_i32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i64_json(msg, INT64_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("9223372036854775807", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(19, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i64_json(msg, &val_i64);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT64(INT64_MAX, val_i64);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i64_json(msg, INT64_MIN);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("-9223372036854775808", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(20, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i64_json(msg, &val_i64);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT64(INT64_MIN, val_i64);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /*
     * String - JSON
     * -------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    const char *exp_str = "Test JSON String";
    ret = ts_msg_add_string_json(msg, exp_str);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_string_json(msg, &val_str, &val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(strlen(exp_str), val_u16);
    TEST_ASSERT_EQUAL_STRING_LEN(exp_str, val_str, val_u16);

    /*
     * Unsigned Integer - JSON
     * -----------------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u16_json(msg, UINT16_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("65535", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(5, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u16_json(msg, &val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, val_u16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u16_json(msg, 0);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("0", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u16_json(msg, &val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(0, val_u16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u32_json(msg, UINT32_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("4294967295", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(10, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u32_json(msg, &val_u32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, val_u32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u32_json(msg, 0);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("0", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u32_json(msg, &val_u32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT32(0, val_u32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u64_json(msg, UINT64_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("18446744073709551615", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(20, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u64_json(msg, &val_u64);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, val_u64);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u64_json(msg, 0);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_STRING_LEN("0", ts_msg_data(msg), ts_msg_len(msg));
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));

    ret = ts_msg_json_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u64_json(msg, &val_u64);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT64(0, val_u64);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /*
     * Array - CBOR
     * -----------------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_array_cbor(msg, 2);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(1, ts_msg_len(msg));

    val_bool = false;
    ret = ts_msg_add_bool_cbor(msg, val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(2, ts_msg_len(msg));

    val_bool = true;
    ret = ts_msg_add_bool_cbor(msg, val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(3, ts_msg_len(msg));

    ret = ts_msg_add_array_end_cbor(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(3, ts_msg_len(msg));

    val_bool = false;
    ret = ts_msg_add_bool_cbor(msg, val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(4, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_bool_cbor(msg, &val_bool);
    TEST_ASSERT_EQUAL(-EINVAL, ret);

    ret = ts_msg_pull_type_cbor(msg, &cbor_type);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(CborArrayType, cbor_type);

    ret = ts_msg_pull_array_cbor(msg, &val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(2, val_u16);

    ret = ts_msg_pull_array_end_cbor(msg);
    TEST_ASSERT_EQUAL(-EAGAIN, ret);

    ret = ts_msg_pull_bool_cbor(msg, &val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(false, val_bool);

    ret = ts_msg_pull_bool_cbor(msg, &val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(true, val_bool);

    ret = ts_msg_pull_array_end_cbor(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_bool_cbor(msg, &val_bool);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(false, val_bool);
    TEST_ASSERT_EQUAL_UINT16(0, ts_msg_len(msg));

    ret = ts_msg_pull_bool_cbor(msg, &val_bool);
    TEST_ASSERT_EQUAL(-ENOMEM, ret);

    /*
     * DecFrac - CBOR
     * --------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_decfrac_cbor(msg, INT32_MAX, INT16_MAX);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_decfrac_cbor(msg, &val_i32, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, val_i32);
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, val_i16);

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_decfrac_cbor(msg, INT32_MIN, INT16_MIN);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_decfrac_cbor(msg, &val_i32, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, val_i32);
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, val_i16);

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_decfrac_cbor(msg, 123, 456);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_decfrac_cbor(msg, &val_i32, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(123, val_i32);
    TEST_ASSERT_EQUAL_INT16(456, val_i16);

    /*
     * Float - CBOR
     * -----------------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    val_u16 = ts_msg_len(msg);
    ret = ts_msg_add_f32_cbor(msg, 123.45678, 5);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_GREATER_THAN_UINT16(val_u16, ts_msg_len(msg));
    TEST_ASSERT_EQUAL_UINT16(5, ts_msg_len(msg));

    val_u16 = ts_msg_len(msg);
    ret = ts_msg_add_f32_cbor(msg, 123.45678, 0);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_GREATER_THAN_UINT16(val_u16, ts_msg_len(msg));
    TEST_ASSERT_EQUAL_UINT16(7, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    val_u16 = ts_msg_len(msg);
    ret = ts_msg_pull_f32_cbor(msg, &val_f32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_FLOAT(123.45678, val_f32);
    TEST_ASSERT_LESS_THAN_UINT16(val_u16, ts_msg_len(msg));

    val_u16 = ts_msg_len(msg);
    ret = ts_msg_pull_f32_cbor(msg, &val_f32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_FLOAT(123.0, val_f32);
    TEST_ASSERT_LESS_THAN_UINT16(val_u16, ts_msg_len(msg));

    /*
     * Signed Integer - CBOR
     * ---------------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i16_cbor(msg, INT16_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(3, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i16_cbor(msg, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT16(INT16_MAX, val_i16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i16_cbor(msg, INT16_MIN);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(3, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i16_cbor(msg, &val_i16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT16(INT16_MIN, val_i16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i32_cbor(msg, INT32_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(5, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i32_cbor(msg, &val_i32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MAX, val_i32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i32_cbor(msg, INT32_MIN);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(5, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i32_cbor(msg, &val_i32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT32(INT32_MIN, val_i32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i64_cbor(msg, INT64_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(9, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i64_cbor(msg, &val_i64);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT64(INT64_MAX, val_i64);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_i64_cbor(msg, INT64_MIN);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(9, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_i64_cbor(msg, &val_i64);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_INT64(INT64_MIN, val_i64);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /*
     * String - CBOR
     * -------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_string_cbor(msg, "12345");
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(6, ts_msg_len(msg));

    ret = ts_msg_add_string_cbor(msg, "67890");
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(12, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_string_cbor(msg, &val_str ,&val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(5, val_u16);
    TEST_ASSERT_EQUAL_STRING_LEN("12345", val_str, 5);

    ret = ts_msg_pull_string_cbor(msg, &val_str ,&val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(5, val_u16);
    TEST_ASSERT_EQUAL_STRING_LEN("67890", val_str, 5);

    /*
     * Byte string - CBOR
     * ------------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_mem_cbor(msg, (uint8_t *)"12345", 5);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(6, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    const uint8_t *val_u8_ptr;
    ret = ts_msg_pull_mem_cbor(msg, &val_u8_ptr ,&val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(5, val_u16);
    TEST_ASSERT_EQUAL_HEX8_ARRAY((uint8_t *)"12345", val_u8_ptr, 5);

    /*
     * Unsigned Integer - CBOR
     * -----------------------
     */

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u16_cbor(msg, UINT16_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(3, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u16_cbor(msg, &val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(UINT16_MAX, val_u16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u16_cbor(msg, 0);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u16_cbor(msg, &val_u16);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(0, val_u16);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u32_cbor(msg, UINT32_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(5, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u32_cbor(msg, &val_u32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, val_u32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u32_cbor(msg, 0);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u32_cbor(msg, &val_u32);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT32(0, val_u32);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u64_cbor(msg, UINT64_MAX);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(9, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u64_cbor(msg, &val_u64);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT64(UINT64_MAX, val_u64);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_reset(msg);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_add_u64_cbor(msg, 0);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));

    ret = ts_msg_cbor_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_u64_cbor(msg, &val_u64);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT64(0, val_u64);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    thingset_msg_unref(msg);
}

void test_msg_add_response_cbor(void)
{
    int ret;
    struct thingset_msg *msg = NULL;
    struct thingset_msg *big_msg = NULL;
    struct thingset_msg *request = NULL;

    ret = thingset_msg_alloc_cbor(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));
    ret = thingset_msg_alloc_cbor(160, 10, &big_msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(big_msg));
    ret = thingset_msg_alloc_cbor(64, 10, &request);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /*
     * ts_msg_add_response_status_cbor
     * -------------------------------
     */

    thingset_msg_reset(msg);

    ret = ts_msg_cbor_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_RESPONSE,
                      TS_MSG_CODE_CHANGED);
    ret = ts_msg_add_response_status_cbor(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT8(TS_MSG_CODE_CHANGED, *ts_msg_data(msg));

    /*
     * ts_msg_add_response_get_cbor
     * ----------------------------
     */

    thingset_msg_reset(request);
    ts_msg_status_set(request, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_REQUEST, 0);

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_CBOR("GET - measurement values",
        msg, request, "meas", 0,
        "h'0000',bin-reponse(h'85'),[14.100000381469727f, 5.130000114440918f, 22]");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_IDS);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_CBOR("GET - measurement values ids",
        msg, request, "meas", 0, "h'0000',bin-reponse(h'85'),[113, 114, 115]");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_CBOR("GET - measurement names/values",
        msg, request, "meas", 0,
        "h'0000',bin-reponse(h'85'),{\"Bat_V\": 14.100000381469727f, "
        "\"Bat_A\": 5.130000114440918f, \"Ambient_degC\": 22}");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_CBOR("GET - missing object - return error",
        msg, request, 0, 0,
        "h'0000',bin-reponse(h'a0')");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_CBOR("GET - object with parent",
        msg, request, ".pub/report", 0,
        "h'0000',bin-reponse(h'85'),{\"Enable\": false, \"Interval_ms\": 1000}");

    /*
     * ts_msg_add_response_fetch_cbor
     * ------------------------------
     *
     * Full round:
     *    ts_msg_add_request_fetch_cbor
     * -> ts_msg_pull_request_cbor
     * -> ts_msg_add_response_fetch_cbor
     */

    const char * tc_response_fetch_cbor_object_names[] = {
        "ui64", "i64", "ui32", "i32", "ui16", "i16", "f32", "bool", "strbuf"
    };
    const char * tc_response_fetch_cbor_float_array_names[] = {
        "arrayfloat",
    };
    const char * tc_response_fetch_cbor_rounded_float_names[] = {
        "f32_rounded",
    };

    /* For testing we need full access authorisation to objects */
    thingset_authorisation_set(TEST_INSTANCE_LOCID, TS_ANY_RW);
    TEST_ASSERT_EQUAL_UINT16(TS_ANY_RW, thingset_authorisation(TEST_INSTANCE_LOCID));

    TEST_ASSERT_MSG_FETCH_CBOR("FETCH meas - ids by id", msg, big_msg,
        "meas",
        true, false, 0, NULL,
        0, "h'0000',bin-fetch(h'05'),2,undefined",
        0, TS_MSG_CODE_REQUEST_FETCH_IDS,
        0, "h'0000',bin-reponse(h'85'),[113, 114, 115]");

    TEST_ASSERT_MSG_FETCH_CBOR("FETCH meas - names by id", msg, big_msg,
        "meas",
        false, false, 0, NULL,
        0, "h'0000',bin-fetch(h'05'),\"meas\",undefined",
        0, TS_MSG_CODE_REQUEST_FETCH_NAMES,
        0, "h'0000',bin-reponse(h'85'),[\"Bat_V\", \"Bat_A\", \"Ambient_degC\"]");

    TEST_ASSERT_MSG_FETCH_CBOR("FETCH conf - single rounded float by id", msg, big_msg,
        "conf",
        true, true, TS_ARRAY_SIZE(tc_response_fetch_cbor_rounded_float_names),
                                                    &tc_response_fetch_cbor_rounded_float_names[0],
        0, "h'0000',bin-fetch(h'05'),6,24586",
        0, TS_MSG_CODE_REQUEST_FETCH_SINGLE,
        0, "h'0000',bin-reponse(h'85'),8");

    TEST_ASSERT_MSG_FETCH_CBOR("FETCH conf - single float array by id", msg, big_msg,
        "conf",
        true, true, TS_ARRAY_SIZE(tc_response_fetch_cbor_float_array_names),
                                                    &tc_response_fetch_cbor_float_array_names[0],
        0, "h'0000',bin-fetch(h'05'),6,28676",
        0, TS_MSG_CODE_REQUEST_FETCH_SINGLE,
        0, "h'0000',bin-reponse(h'85'),[2.2699999809265137f, 3.440000057220459f]");

    TEST_ASSERT_MSG_FETCH_CBOR("FETCH conf - multiple objects by id", msg, big_msg,
        "conf",
        true, true, TS_ARRAY_SIZE(tc_response_fetch_cbor_object_names),
                                                       &tc_response_fetch_cbor_object_names[0],
        0, "h'0000',"
           "bin-fetch(h'05'),6,[24577, 24578, 24579, 24580, 24581, 24582, 24583, 24584, 24585]",
        0, TS_MSG_CODE_REQUEST_FETCH_VALUES,
        0, "h'0000',"
           "bin-reponse(h'85'),[1, 2, 3, 4, 5, 6, 8.3999996185302734f, true, \"test\"]");

    TEST_ASSERT_MSG_FETCH_CBOR("FETCH conf - multiple objects by name", msg, big_msg,
        "conf",
        false, false, TS_ARRAY_SIZE(tc_response_fetch_cbor_object_names),
                                                       &tc_response_fetch_cbor_object_names[0],
        0, "h'0000',"
           "bin-fetch(h'05'),\"conf\",[\"ui64\", \"i64\", \"ui32\", \"i32\", \"ui16\", "
                                      "\"i16\", \"f32\", \"bool\", \"strbuf\"]",
        0, TS_MSG_CODE_REQUEST_FETCH_VALUES,
        0, "h'0000',"
           "bin-reponse(h'85'),[1, 2, 3, 4, 5, 6, 8.3999996185302734f, true, \"test\"]");

    thingset_msg_unref(msg);
    thingset_msg_unref(big_msg);
    thingset_msg_unref(request);
}

void test_msg_add_response_json(void)
{
    int ret;
    struct thingset_msg *msg = NULL;
    struct thingset_msg *request = NULL;
    thingset_oref_t object_oref;
    const char *req_str;
    const char *resp_str;

    object_oref = ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID));

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    ret = thingset_msg_alloc_json(64, 10, &request);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(request));


    /*
     * ts_msg_add_response_status_json
     * -------------------------------
     */

    thingset_msg_reset(msg);

    ret = ts_msg_json_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);

    const struct {
        ts_msg_status_code_t code;
        const char *expected;
    } tc_response_status_json[] = {
#if TS_CONFIG_VERBOSE_STATUS_MESSAGES
        {.code = TS_STATUS_CREATED, .expected = ":81 Created."},
        {.code = TS_STATUS_DELETED, .expected = ":82 Deleted."},
        {.code = TS_STATUS_VALID, .expected = ":83 Valid."},
        {.code = TS_STATUS_CHANGED, .expected = ":84 Changed."},
        {.code = TS_STATUS_CONTENT, .expected = ":85 Content."},
        {.code = TS_STATUS_BAD_REQUEST, .expected = ":A0 Bad Request."},
        {.code = TS_STATUS_UNAUTHORIZED, .expected = ":A1 Unauthorized."},
        {.code = TS_STATUS_FORBIDDEN, .expected = ":A3 Forbidden."},
        {.code = TS_STATUS_NOT_FOUND, .expected = ":A4 Not Found."},
        {.code = TS_STATUS_METHOD_NOT_ALLOWED, .expected = ":A5 Method Not Allowed."},
        {.code = TS_STATUS_REQUEST_INCOMPLETE, .expected = ":A8 Request Entity Incomplete."},
        {.code = TS_STATUS_CONFLICT, .expected = ":A9 Conflict."},
        {.code = TS_STATUS_REQUEST_TOO_LARGE, .expected = ":AD Request Entity Too Large."},
        {.code = TS_STATUS_UNSUPPORTED_FORMAT, .expected = ":AF Unsupported Content-Format."},
        {.code = TS_STATUS_INTERNAL_SERVER_ERR, .expected = ":C0 Internal Server Error."},
        {.code = TS_STATUS_NOT_IMPLEMENTED, .expected = ":C1 Not Implemented."},
        {.code = TS_STATUS_RESPONSE_TOO_LARGE, .expected = ":E1 Response too large."},
        {.code = 0x0FF, .expected = ":FF Error."},
#else
        {.code = TS_STATUS_CREATED, .expected = ":81."},
        {.code = TS_STATUS_DELETED, .expected = ":82."},
        {.code = TS_STATUS_VALID, .expected = ":83."},
        {.code = TS_STATUS_CHANGED, .expected = ":84."},
        {.code = TS_STATUS_CONTENT, .expected = ":85."},
        {.code = TS_STATUS_BAD_REQUEST, .expected = ":A0."},
        {.code = TS_STATUS_UNAUTHORIZED, .expected = ":A1."},
        {.code = TS_STATUS_FORBIDDEN, .expected = ":A3."},
        {.code = TS_STATUS_NOT_FOUND, .expected = ":A4."},
        {.code = TS_STATUS_METHOD_NOT_ALLOWED, .expected = ":A5."},
        {.code = TS_STATUS_REQUEST_INCOMPLETE, .expected = ":A8."},
        {.code = TS_STATUS_CONFLICT, .expected = ":A9."},
        {.code = TS_STATUS_REQUEST_TOO_LARGE, .expected = ":AD."},
        {.code = TS_STATUS_UNSUPPORTED_FORMAT, .expected = ":AF."},
        {.code = TS_STATUS_INTERNAL_SERVER_ERR, .expected = ":C0."},
        {.code = TS_STATUS_NOT_IMPLEMENTED, .expected = ":C1."},
        {.code = TS_STATUS_RESPONSE_TOO_LARGE, .expected = ":E1."},
        {.code = 0x0FF, .expected = ":FF."},
#endif
    };

    for (int i = 0; i < TS_ARRAY_SIZE(tc_response_status_json); i++) {
        thingset_msg_reset(msg);
        TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(msg), tc_response_status_json[i].expected);

        ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_RESPONSE,
                          tc_response_status_json[i].code);
        ret = ts_msg_add_response_status_json(msg);
        TEST_ASSERT_EQUAL_MESSAGE(0, ret, tc_response_status_json[i].expected);
        TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(tc_response_status_json[i].expected, ts_msg_data(msg),
                                             ts_msg_len(msg), tc_response_status_json[i].expected);
        TEST_ASSERT_EQUAL_MESSAGE(strlen(tc_response_status_json[i].expected), ts_msg_len(msg),
                                  tc_response_status_json[i].expected);
    }

    /*
     * ts_msg_add_response_get_json
     * ----------------------------
     */

    thingset_msg_reset(msg);
    thingset_msg_reset(request);

    ts_msg_status_set(request, TS_MSG_VALID_OK, TS_MSG_PROTO_BIN, TS_MSG_TYPE_REQUEST, 0);

#if TS_CONFIG_VERBOSE_STATUS_MESSAGES
    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON("GET values - DeviceID",
        msg, request, 0x1b, 0,
        ":85 Content. \"" TEST_DEVICE_ID_INSTANCE "\"");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON("GET values - meas",
        msg, request, ID_MEAS, 0,
        ":85 Content. [14.10,5.13,22]");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_NAMES_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON("GET names & values - meas",
        msg, request, ID_MEAS, 0,
        ":85 Content. {\"Bat_V\":14.10,\"Bat_A\":5.13,\"Ambient_degC\":22}");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON("GET values - report",
        msg, request, ID_REPORT, 0,
        ":85 Content. [\"Timestamp_s\",\"Bat_V\",\"Bat_A\",\"Ambient_degC\"]");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON("GET values - .pub -> bad request",
        msg, request, ID_PUB, 0,
        ":A0 Bad Request.");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON("GET values - x-reset -> bad request",
        msg, request, 0xE1, 0,
        ":A0 Bad Request.");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON("GET values - arrayi32",
        msg, request, 0x7003, 0,
        ":85 Content. [4,2,8,4]");

    ts_msg_status_code_set(request, TS_MSG_CODE_REQUEST_GET_VALUES);
    TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON("GET values - arrayfloat",
        msg, request, 0x7004, 0,
        ":85 Content. [2.27,3.44]");
#else
#error "Not Supported"
#endif

    /* Full round - ts_msg_pull_request_json -> ts_msg_add_response_get_json */

    thingset_msg_reset(msg);
    thingset_msg_reset(request);

    req_str = "?meas";
    resp_str = ":85 Content. {\"Bat_V\":14.10,\"Bat_A\":5.13,\"Ambient_degC\":22}";

    ts_msg_status_set(request, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    ret = ts_msg_add_mem(request, (const uint8_t *)req_str, strlen(req_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(request, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(request, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(request));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(request));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(request));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_NAMES_VALUES, ts_msg_status_code(request));
    TEST_ASSERT_EQUAL_UINT16(TS_EXPAND(TS_ANY_RW), ts_msg_auth(request));

    ret = ts_msg_add_response_get_json(msg, object_oref, request);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(strlen(resp_str), ts_msg_len(msg));
    TEST_ASSERT_EQUAL_STRING_LEN(resp_str, ts_msg_data(msg), strlen(resp_str));

    /*
     * ts_msg_add_response_fetch_json
     * ------------------------------
     */

    /* Full round - ts_msg_pull_request_json -> ts_msg_add_response_fetch_json */

    thingset_msg_reset(msg);
    thingset_msg_reset(request);

    f32 = 52.80;
    b = false;
    i32 = 50;
    req_str = "?conf [\"f32\",\"bool\",\"i32\"]";
    resp_str = ":85 Content. [52.80,false,50]";

    ts_msg_status_set(request, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    ret = ts_msg_add_mem(request, (const uint8_t *)req_str, strlen(req_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(request, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(request, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(request));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(request));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(request));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_FETCH_VALUES, ts_msg_status_code(request));
    TEST_ASSERT_EQUAL_UINT16(TS_ANY_RW, ts_msg_auth(request));

    const char *log_str = "h'0707',txt-fetch(\"?\"),\"conf\","
                          "[\"f32\",\"bool\",\"i32\"]";
    ret = ts_msg_log(request, &test_log_buf[0], sizeof(test_log_buf));
    TEST_ASSERT_EQUAL_STRING_LEN(log_str, &test_log_buf[ASSERT_MSG_LOG_SKIP], ret);

    ret = ts_msg_add_response_fetch_json(msg, object_oref, request);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL_UINT16(strlen(resp_str), ts_msg_len(msg));
    TEST_ASSERT_EQUAL_STRING_LEN(resp_str, ts_msg_data(msg), strlen(resp_str));

    log_str = "h'0000',txt-reponse(\":\"),h'85',\"Content.\","
              "[52.80,false,50]";
    ret = ts_msg_log(msg, &test_log_buf[0], sizeof(test_log_buf));
    TEST_ASSERT_EQUAL_STRING_LEN(log_str, &test_log_buf[ASSERT_MSG_LOG_SKIP], ret);
}

void test_msg_add_statement_cbor(void)
{
    int ret;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;
    char tc_desc[300];

    object_oref = ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID));

    ret = thingset_msg_alloc_cbor(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    const struct {
        const char *tc;
        const char *object_path;
        int expected_ret;
        const char *expected_str;
    } tc_add_statement_cbor[] = {
        {   .tc = "STATEMENT subset - report",
            .object_path = "report",
            .expected_ret = 0,
            .expected_str = "h'0000',bin-statement(h'1f'),10,"
                            "[12345678, 14.100000381469727f, 5.130000114440918f, 22]" },
        {   .tc = "STATEMENT group - info",
            .object_path = "info",
            .expected_ret = 0,
            .expected_str = "h'0000',bin-statement(h'1f'),1,"
                            "[\"Libre Solar\", 12345678, \"ABCD1234\"]" },
        {   .tc = "STATEMENT missing object - error",
            .object_path = 0,
            .expected_ret = -EINVAL,
            .expected_str = 0 },
        {   .tc = "STATEMENT object with parent - not supported, silently ignore",
            .object_path = ".pub/report",
            .expected_ret = 0,
            .expected_str = 0 },
    };

    for (int i = 0; i < TS_ARRAY_SIZE(tc_add_statement_cbor); i++) {
        /* Prepare assert message */
        int len = snprintf(&tc_desc[0], sizeof(tc_desc) - 1,
                          "TC#%.2d %s", i, tc_add_statement_cbor[i].tc);

        thingset_msg_reset(msg);
        TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(msg), &tc_desc[0]);
        ts_msg_log(msg, &test_log_buf[0], sizeof(test_log_buf));
        TEST_ASSERT_EQUAL_STRING_MESSAGE("h'0000',error(\"initial command missing.\")",
                                         &test_log_buf[ASSERT_MSG_LOG_SKIP], &tc_desc[0]);

        if (tc_add_statement_cbor[i].object_path == NULL) {
#if TS_CONFIG_ASSERT
            /* Can not be tested as assertion will be triggered */
            continue;
#endif
            object_oref = ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID));
        }
        else {
            ret = ts_obj_by_path(ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID)),
                                 tc_add_statement_cbor[i].object_path,
                                 (size_t)strlen(tc_add_statement_cbor[i].object_path),
                                 &object_oref);
            TEST_ASSERT_EQUAL_MESSAGE(0, ret, &tc_desc[0]);
        }

        ret = ts_msg_add_statement_cbor(msg, object_oref);
        ts_msg_log(msg, &test_log_buf[0], sizeof(test_log_buf));

        /* Complete assert message */
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
        snprintf(&tc_desc[len], sizeof(tc_desc) - len - 1,
            " - '%s', id: %u, type: %u, parent: %u -> %s", object_name, (unsigned int)object_id,
            (unsigned int)object_type, (unsigned int)object_parent_id, &test_log_buf[0]);

        TEST_ASSERT_EQUAL_MESSAGE(tc_add_statement_cbor[i].expected_ret, ret, &tc_desc[0]);
        if (tc_add_statement_cbor[i].expected_str != NULL) {
            TEST_ASSERT_EQUAL_STRING_MESSAGE(tc_add_statement_cbor[i].expected_str,
                                             &test_log_buf[ASSERT_MSG_LOG_SKIP], &tc_desc[0]);
        }
        else {
            TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(msg), &tc_desc[0]);
        }
    }
}

void test_msg_add_statement_json(void)
{
    int ret;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;
    char tc_desc[300];

    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    const struct {
        const char *tc;
        const char *object_path;
        int expected_ret;
        const char *expected_str;
    } tc_add_statement_json[] = {
        {   .tc = "STATEMENT subset - report",
            .object_path = "report",
            .expected_ret = 0,
            .expected_str = "#report {\"Timestamp_s\":12345678,\"Bat_V\":14.10,\"Bat_A\":5.13,\"Ambient_degC\":22}" },
        {   .tc = "STATEMENT group - info",
            .object_path = "info",
            .expected_ret = 0,
            .expected_str = "#info {\"Manufacturer\":\"Libre Solar\",\"Timestamp_s\":12345678,\"DeviceID\":\"ABCD1234\"}" },
        {   .tc = "STATEMENT missing object - error",
            .object_path = 0,
            .expected_ret = -EINVAL,
            .expected_str = 0 },
        {   .tc = "STATEMENT object with parent - not supported, silently ignore",
            .object_path = ".pub/report",
            .expected_ret = 0,
            .expected_str = 0 },
    };

    for (int i = 0; i < TS_ARRAY_SIZE(tc_add_statement_json); i++) {
        /* Prepare assert message */
        int len = snprintf(&tc_desc[0], sizeof(tc_desc) - 1,
            "TC#%.2d %s", i, tc_add_statement_json[i].tc);

        thingset_msg_reset(msg);
        TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(msg), &tc_desc[0]);

        if (tc_add_statement_json[i].object_path == NULL) {
#if TS_CONFIG_ASSERT
            /* Can not be tested as assertion will be triggered */
            continue;
#endif
            object_oref = ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID));
        }
        else {
            ret = ts_obj_by_path(ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID)),
                                 tc_add_statement_json[i].object_path,
                                 (size_t)strlen(tc_add_statement_json[i].object_path),
                                 &object_oref);
            TEST_ASSERT_EQUAL_MESSAGE(0, ret, &tc_desc[0]);
        }

        ret = ts_msg_add_statement_json(msg, object_oref);

        /* Complete assert message */
        const char *object_name = "<none>";
        ts_obj_id_t object_parent_id = 0;
        uint8_t object_type = 0;
        if (ts_obj_db_oref_is_object(object_oref)) {
            object_name = ts_obj_name(object_oref);
            object_parent_id = ts_obj_parent_id(object_oref);
            object_type = ts_obj_type(object_oref);
        }
        snprintf(&tc_desc[len], sizeof(tc_desc) - len - 1,
            " - '%s', type: %u, parent: %u -> %.*s", object_name, (unsigned int)object_type,
            (unsigned int)object_parent_id, (int)ts_msg_len(msg), (const char *)ts_msg_data(msg));

        TEST_ASSERT_EQUAL_MESSAGE(tc_add_statement_json[i].expected_ret, ret, &tc_desc[0]);
        if (tc_add_statement_json[i].expected_str != NULL) {
            TEST_ASSERT_EQUAL_MESSAGE(strlen(tc_add_statement_json[i].expected_str),
                                      ts_msg_len(msg), &tc_desc[0]);
            TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(tc_add_statement_json[i].expected_str,
                                                 ts_msg_data(msg), ts_msg_len(msg), &tc_desc[0]);
        }
        else {
            TEST_ASSERT_EQUAL_MESSAGE(0, ts_msg_len(msg), &tc_desc[0]);
        }
    }

    thingset_msg_unref(msg);
}

/**
 * @brief Test message COBS (en-/de)coding
 *
 * This test verifies message buffer usage:
 * - ts_msg_cobs_enc_setup()
 * - ts_msg_cobs_dec_setup()
 */
void test_msg_cobs(void)
{
    int ret;
    struct thingset_msg *msg = NULL;
    uint8_t buf[TS_COBS_INPLACE_SAFE_BUFFER_SIZE];

    TEST_ASSERT_EQUAL(256, TS_COBS_INPLACE_SAFE_BUFFER_SIZE);

    ret = thingset_msg_alloc(TS_COBS_INPLACE_SAFE_BUFFER_SIZE, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* Empty */
    ret = ts_msg_cobs_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(2, ts_msg_len(msg));
    TEST_ASSERT_EQUAL_HEX8_ARRAY("\x01\x00", ts_msg_data(msg), ts_msg_len(msg));
    ret = ts_msg_cobs_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* One non-zero byte */
    thingset_msg_reset(msg);
    ts_msg_add_u8(msg, 0x01);
    ret = ts_msg_cobs_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(3, ts_msg_len(msg));
    TEST_ASSERT_EQUAL_HEX8_ARRAY("\x02\x01\x00", ts_msg_data(msg), ts_msg_len(msg));
    ret = ts_msg_cobs_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));
    TEST_ASSERT_EQUAL_HEX8_ARRAY("\x01", ts_msg_data(msg), ts_msg_len(msg));

    /* One zero byte */
    thingset_msg_reset(msg);
    ts_msg_add_u8(msg, 0x00);
    ret = ts_msg_cobs_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(3, ts_msg_len(msg));
    TEST_ASSERT_EQUAL_HEX8_ARRAY("\x01\x01\x00", ts_msg_data(msg), ts_msg_len(msg));
    ret = ts_msg_cobs_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(1, ts_msg_len(msg));
    TEST_ASSERT_EQUAL_HEX8_ARRAY("\x00", ts_msg_data(msg), ts_msg_len(msg));

    /* Longest possible run of 254 bytes */
    thingset_msg_reset(msg);
    for (int i = 0; i < 254; i++) {
        buf[i] = 1;
    }
    ret = ts_msg_add_mem(msg, &buf[0], 254);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(254, ts_msg_len(msg));
    ret = ts_msg_cobs_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(256, ts_msg_len(msg));
    ret = ts_msg_cobs_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(254, ts_msg_len(msg));

    /* Safe payload, all zero bytes */
    thingset_msg_reset(msg);
    for (int i = 0; i < 254; i++) {
        buf[i] = 0;
    }
    ret = ts_msg_add_mem(msg, &buf[0], 254);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(254, ts_msg_len(msg));
    ret = ts_msg_cobs_enc_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(256, ts_msg_len(msg));
    ret = ts_msg_cobs_dec_setup(msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(254, ts_msg_len(msg));
}

void test_msg_pull_request_get_cbor(void)
{
    int ret;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;

    object_oref = ts_obj_db_oref_any(ts_ctx_obj_db(TEST_INSTANCE_LOCID));

    ret = thingset_msg_alloc_cbor(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* Get root object - names by object path */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_cbor(msg, 0, "/");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_cbor(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_BIN, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_NAMES, ts_msg_status_code(msg));
    TEST_ASSERT_EQUAL_UINT16(TEST_INSTANCE_LOCID, object_oref.db_id);
    TEST_ASSERT_EQUAL_UINT16(TS_OBJ_DB_OID_ROOT, object_oref.db_oid);
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL(0, ts_obj_id(object_oref));

    /* Get root object - error on invalid path by object path */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_cbor(msg, 0, "/invalid");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_cbor(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_BIN, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* Get root object - ids and values by object id */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_cbor(msg, 0, NULL);
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_cbor(msg, &object_oref);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_BIN, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_IDS_VALUES, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL(0, ts_obj_id(object_oref));

    /* Get 'meas' object - names and values by object path */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_cbor(msg, 0, "meas");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_cbor(msg, &object_oref);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_BIN, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_NAMES_VALUES, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL(ID_MEAS, ts_obj_id(object_oref));

    /* Get 'meas' object - ids and values by object id */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_cbor(msg, ID_MEAS, NULL);
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_cbor(msg, &object_oref);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_BIN, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_IDS_VALUES, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL(ID_MEAS, ts_obj_id(object_oref));

    /* Get uknown object - error on unknown path by object path */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_cbor(msg, 0, "unknown");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_cbor(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_BIN, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_NOT_FOUND, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    thingset_msg_unref(msg);
}

void test_msg_pull_request_get_json(void)
{
    int ret;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;

    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* GET root names */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_json(msg, "/");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_NAMES, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("/", ts_obj_name(object_oref));

    /* GET names */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_json(msg, "meas/");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_NAMES, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("meas", ts_obj_name(object_oref));

    /* GET names on external object - bad request */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_json(msg, "meas/Bat_V/");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* GET names&values */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_json(msg, "meas");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_NAMES_VALUES, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("meas", ts_obj_name(object_oref));

    /* GET single value */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_json(msg, "meas/Bat_V");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_GET_NAMES_VALUES, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("Bat_V", ts_obj_name(object_oref));

    /* GET names on external object - bad request */
    thingset_msg_reset(msg);

    ret = ts_msg_add_request_get_json(msg, "rpc/x-dummy");
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));
}

void test_msg_pull_request_fetch_json(void)
{
    int ret;
    const char *request_str;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;

    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* FETCH array */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "?conf [\"f32\",\"bool\",\"i32\"]";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_FETCH_VALUES, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("conf", ts_obj_name(object_oref));
}

void test_msg_pull_request_patch_json(void)
{
    int ret;
    const char *request_str;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;

    /* Tests work on instance context database */
    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* PATCH array */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "=conf {\"f32\" : 52.8,\"i32\":50.6}";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_PATCH, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("conf", ts_obj_name(object_oref));

    /* PATCH missing object - bad request */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "={\"f32\" : 52.8,\"i32\":50.6}";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* PATCH invalid object - bad request */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "=/{\"f32\" : 52.8,\"i32\":50.6}";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* PATCH missing data structure - bad request */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "=conf";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* PATCH wrongly formated data structure - bad request */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "=conf [\"f32\":54.3";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* PATCH wrong data structure - not detected here */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "=conf{\"f32\":54.3}";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_PATCH, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("conf", ts_obj_name(object_oref));

    /* Tests work on neighbour context database - all data objects are children of root */
    ts_obj_db_oref_init(TEST_NEIGHBOUR_LOCID, &object_oref);

    /* Patch with default root path */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "= {\"HeaterEnable\":true}";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_PATCH, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("/", ts_obj_name(object_oref));
}

void test_msg_pull_request_exec_json(void)
{
    int ret;
    const char *request_str;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;

    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* EXEC */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "!rpc/x-dummy";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_EXEC, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("x-dummy", ts_obj_name(object_oref));

    /* EXEC not authorized - unauthorized */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "!rpc/x-dummy";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, TS_ANY_R);

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_UNAUTHORIZED, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* EXEC unknown object - not found */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "!rpc/x-unknown";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_NOT_FOUND, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* EXEC invalid object - bad request */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "!meas/Bat_V";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));
}

void test_msg_pull_request_create_json(void)
{
    int ret;
    const char *request_str;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;

    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* CREATE object */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "+report \"Ambient_degC\"";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_CREATE, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("report", ts_obj_name(object_oref));

    /* CREATE missing data structure - bad request */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "+report                ";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));
}

void test_msg_pull_request_delete_json(void)
{
    int ret;
    const char *request_str;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;

    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* DELETE object */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "-report \"Ambient_degC\"";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_OK, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_REQUEST_DELETE, ts_msg_status_code(msg));
    TEST_ASSERT_TRUE(ts_obj_db_oref_is_valid(object_oref));
    TEST_ASSERT_EQUAL_STRING("report", ts_obj_name(object_oref));

    /* DELETE missing data structure - bad request */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "-report                ";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));
}

void test_msg_pull_request_invalid_json(void)
{
    int ret;
    const char *request_str;
    struct thingset_msg *msg = NULL;
    thingset_oref_t object_oref;

    ts_obj_db_oref_init(TEST_INSTANCE_LOCID, &object_oref);

    ret = thingset_msg_alloc_json(64, 10, &msg);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(0, ts_msg_len(msg));

    /* Invalid command - bad request */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    request_str = "$meas/Bat_V";
    ret = ts_msg_add_mem(msg, request_str, strlen(request_str));
    TEST_ASSERT_EQUAL(0, ret);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));

    /* Empty request - return != 0 */
    thingset_msg_reset(msg);

    ts_msg_status_set(msg, TS_MSG_VALID_OK, TS_MSG_PROTO_TXT, TS_MSG_TYPE_REQUEST, 0);
    ts_msg_auth_set(msg, thingset_authorisation(TEST_INSTANCE_LOCID));

    ret = ts_msg_pull_request_json(msg, &object_oref);
    TEST_ASSERT_EQUAL(0, ret);
    TEST_ASSERT_EQUAL(TS_MSG_VALID_ERROR, ts_msg_status_valid(msg));
    TEST_ASSERT_EQUAL(TS_MSG_PROTO_TXT, ts_msg_status_proto(msg));
    TEST_ASSERT_EQUAL(TS_MSG_TYPE_REQUEST, ts_msg_status_type(msg));
    TEST_ASSERT_EQUAL(TS_MSG_CODE_BAD_REQUEST, ts_msg_status_code(msg));
    TEST_ASSERT_FALSE(ts_obj_db_oref_is_valid(object_oref));
}

void tests_msg(void)
{
    UNITY_BEGIN();

    /*  Message buffer support */
    RUN_TEST(test_msg_alloc);
    RUN_TEST(test_msg_add_primitives);
    RUN_TEST(test_msg_add_response_cbor);
    RUN_TEST(test_msg_add_response_json);
    RUN_TEST(test_msg_add_statement_cbor);
    RUN_TEST(test_msg_add_statement_json);
    RUN_TEST(test_msg_cobs);
    RUN_TEST(test_msg_pull_request_get_cbor);
    RUN_TEST(test_msg_pull_request_get_json);
    RUN_TEST(test_msg_pull_request_fetch_json);
    RUN_TEST(test_msg_pull_request_patch_json);
    RUN_TEST(test_msg_pull_request_exec_json);
    RUN_TEST(test_msg_pull_request_create_json);
    RUN_TEST(test_msg_pull_request_delete_json);
    RUN_TEST(test_msg_pull_request_invalid_json);

    UNITY_END();
}
