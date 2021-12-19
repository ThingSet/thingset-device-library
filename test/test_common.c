/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

/**
 * @brief Test conversion json to cbor.
 */
void test_txt_patch_bin_fetch(void)
{
    /* Assure core context correctly initialized for testing */
    TEST_ASSERT_EQUAL_UINT8(0, TS_CONFIG_CORE_LOCID);
    TEST_ASSERT_EQUAL_UINT8(0, TEST_CORE_LOCID);
    TEST_ASSERT_EQUAL_UINT16(TS_ANY_RW, thingset_authorisation(TEST_CORE_LOCID));

    // uint16
    TEST_ASSERT_JSON2CBOR("ui16", "0", 0x6005, "00");
    TEST_ASSERT_JSON2CBOR("ui16", "23", 0x6005, "17");
    TEST_ASSERT_JSON2CBOR("ui16", "24", 0x6005, "18 18");
    TEST_ASSERT_JSON2CBOR("ui16", "255", 0x6005, "18 ff");
    TEST_ASSERT_JSON2CBOR("ui16", "256", 0x6005, "19 01 00");
    TEST_ASSERT_JSON2CBOR("ui16", "65535", 0x6005, "19 FF FF");

    // uint32
    TEST_ASSERT_JSON2CBOR("ui32", "0", 0x6003, "00");
    TEST_ASSERT_JSON2CBOR("ui32", "23", 0x6003, "17");
    TEST_ASSERT_JSON2CBOR("ui32", "24", 0x6003, "18 18");
    TEST_ASSERT_JSON2CBOR("ui32", "255", 0x6003, "18 ff");
    TEST_ASSERT_JSON2CBOR("ui32", "256", 0x6003, "19 01 00");
    TEST_ASSERT_JSON2CBOR("ui32", "65535", 0x6003, "19 FF FF");
    TEST_ASSERT_JSON2CBOR("ui32", "65536", 0x6003, "1A 00 01 00 00");
    TEST_ASSERT_JSON2CBOR("ui32", "4294967295", 0x6003, "1A FF FF FF FF");

#if TS_CONFIG_64BIT_TYPES_SUPPORT
    // uint64
    TEST_ASSERT_JSON2CBOR("ui64", "4294967295", 0x6001, "1A FF FF FF FF");
    TEST_ASSERT_JSON2CBOR("ui64", "4294967296", 0x6001, "1B 00 00 00 01 00 00 00 00");
    TEST_ASSERT_JSON2CBOR("ui64", "9223372036854775807", 0x6001, "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // int16 (positive values)
    TEST_ASSERT_JSON2CBOR("i16", "0", 0x6006, "00");
    TEST_ASSERT_JSON2CBOR("i16", "23", 0x6006, "17");
    TEST_ASSERT_JSON2CBOR("i16", "24", 0x6006, "18 18");
    TEST_ASSERT_JSON2CBOR("i16", "255", 0x6006, "18 ff");
    TEST_ASSERT_JSON2CBOR("i16", "256", 0x6006, "19 01 00");
    TEST_ASSERT_JSON2CBOR("i16", "32767", 0x6006, "19 7F FF");                 // maximum value for int16

    // int32 (positive values)
    TEST_ASSERT_JSON2CBOR("i32", "0", 0x6004, "00");
    TEST_ASSERT_JSON2CBOR("i32", "23", 0x6004, "17");
    TEST_ASSERT_JSON2CBOR("i32", "24", 0x6004, "18 18");
    TEST_ASSERT_JSON2CBOR("i32", "255", 0x6004, "18 ff");
    TEST_ASSERT_JSON2CBOR("i32", "256", 0x6004, "19 01 00");
    TEST_ASSERT_JSON2CBOR("i32", "65535", 0x6004, "19 FF FF");
    TEST_ASSERT_JSON2CBOR("i32", "65536", 0x6004, "1A 00 01 00 00");
    TEST_ASSERT_JSON2CBOR("i32", "2147483647", 0x6004, "1A 7F FF FF FF");      // maximum value for int32

#if TS_CONFIG_64BIT_TYPES_SUPPORT
    // int64 (positive values)
    TEST_ASSERT_JSON2CBOR("i64", "4294967295", 0x6002, "1A FF FF FF FF");
    TEST_ASSERT_JSON2CBOR("i64", "4294967296", 0x6002, "1B 00 00 00 01 00 00 00 00");
    TEST_ASSERT_JSON2CBOR("i64", "9223372036854775807", 0x6002, "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // int16 (negative values)
    TEST_ASSERT_JSON2CBOR("i16", "-0", 0x6006, "00");
    TEST_ASSERT_JSON2CBOR("i16", "-24", 0x6006, "37");
    TEST_ASSERT_JSON2CBOR("i16", "-25", 0x6006, "38 18");
    TEST_ASSERT_JSON2CBOR("i16", "-256", 0x6006, "38 ff");
    TEST_ASSERT_JSON2CBOR("i16", "-257", 0x6006, "39 01 00");
    TEST_ASSERT_JSON2CBOR("i16", "-32768", 0x6006, "39 7F FF");

    // int32 (negative values)
    TEST_ASSERT_JSON2CBOR("i32", "-0", 0x6004, "00");
    TEST_ASSERT_JSON2CBOR("i32", "-24", 0x6004, "37");
    TEST_ASSERT_JSON2CBOR("i32", "-25", 0x6004, "38 18");
    TEST_ASSERT_JSON2CBOR("i32", "-256", 0x6004, "38 ff");
    TEST_ASSERT_JSON2CBOR("i32", "-257", 0x6004, "39 01 00");
    TEST_ASSERT_JSON2CBOR("i32", "-65536", 0x6004, "39 FF FF");
    TEST_ASSERT_JSON2CBOR("i32", "-65537", 0x6004, "3A 00 01 00 00");
    TEST_ASSERT_JSON2CBOR("i32", "-2147483648", 0x6004, "3A 7F FF FF FF");      // maximum value for int32

#if TS_CONFIG_64BIT_TYPES_SUPPORT
    // int64 (negative values)
    TEST_ASSERT_JSON2CBOR("i64", "-4294967296", 0x6002, "3A FF FF FF FF");
    TEST_ASSERT_JSON2CBOR("i64", "-4294967297", 0x6002, "3B 00 00 00 01 00 00 00 00");
    TEST_ASSERT_JSON2CBOR("i64", "-9223372036854775808", 0x6002, "3B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // float
    TEST_ASSERT_JSON2CBOR("f32", "12.340",  0x6007, "fa 41 45 70 a4");
    TEST_ASSERT_JSON2CBOR("f32", "-12.340", 0x6007, "fa c1 45 70 a4");
    TEST_ASSERT_JSON2CBOR("f32", "12.345",  0x6007, "fa 41 45 85 1f");

#if TS_CONFIG_DECFRAC_TYPE_SUPPORT
    // decimal fraction
    TEST_ASSERT_JSON2CBOR("DecFrac_degC", "27315e-2", 0x600B, "c4 82 21 19 6a b3");
    TEST_ASSERT_JSON2CBOR("DecFrac_degC", "273.15", 0x600B, "c4 82 21 19 6a b3");
#endif

    // bool
    TEST_ASSERT_JSON2CBOR("bool", "true",  0x6008, "f5");
    TEST_ASSERT_JSON2CBOR("bool", "false",  0x6008, "f4");

    // string
    TEST_ASSERT_JSON2CBOR("strbuf", "\"Test\"",  0x6009, "64 54 65 73 74");
    TEST_ASSERT_JSON2CBOR("strbuf", "\"Hello World!\"",  0x6009, "6c 48 65 6c 6c 6f 20 57 6f 72 6c 64 21");
}

/**
 * @brief Test conversion json to cbor.
 */
void test_bin_patch_txt_fetch(void)
{
    /* Assure core context correctly initialized for testing */
    TEST_ASSERT_EQUAL_UINT16(TS_ANY_RW, thingset_authorisation(TEST_CORE_LOCID));

    // uint16
    TEST_ASSERT_CBOR2JSON("ui16", "0", 0x6005, "00");
    TEST_ASSERT_CBOR2JSON("ui16", "23", 0x6005, "17");
    TEST_ASSERT_CBOR2JSON("ui16", "23", 0x6005, "18 17");       // less compact format
    TEST_ASSERT_CBOR2JSON("ui16", "24", 0x6005, "18 18");
    TEST_ASSERT_CBOR2JSON("ui16", "255", 0x6005, "18 ff");
    TEST_ASSERT_CBOR2JSON("ui16", "255", 0x6005, "19 00 ff");   // less compact format
    TEST_ASSERT_CBOR2JSON("ui16", "256", 0x6005, "19 01 00");
    TEST_ASSERT_CBOR2JSON("ui16", "65535", 0x6005, "19 FF FF");

    // uint32
    TEST_ASSERT_CBOR2JSON("ui32", "0", 0x6003, "00");
    TEST_ASSERT_CBOR2JSON("ui32", "23", 0x6003, "17");
    TEST_ASSERT_CBOR2JSON("ui32", "23", 0x6003, "18 17");       // less compact format
    TEST_ASSERT_CBOR2JSON("ui32", "24", 0x6003, "18 18");
    TEST_ASSERT_CBOR2JSON("ui32", "255", 0x6003, "18 ff");
    TEST_ASSERT_CBOR2JSON("ui32", "255", 0x6003, "19 00 ff");   // less compact format
    TEST_ASSERT_CBOR2JSON("ui32", "256", 0x6003, "19 01 00");
    TEST_ASSERT_CBOR2JSON("ui32", "65535", 0x6003, "19 FF FF");
    TEST_ASSERT_CBOR2JSON("ui32", "65535", 0x6003, "1A 00 00 FF FF");  // less compact format
    TEST_ASSERT_CBOR2JSON("ui32", "65536", 0x6003, "1A 00 01 00 00");
    TEST_ASSERT_CBOR2JSON("ui32", "4294967295", 0x6003, "1A FF FF FF FF");

#if TS_CONFIG_64BIT_TYPES_SUPPORT
    // uint64
    TEST_ASSERT_CBOR2JSON("ui64", "4294967295", 0x6001, "1A FF FF FF FF");
    TEST_ASSERT_CBOR2JSON("ui64", "4294967295", 0x6001, "1B 00 00 00 00 FF FF FF FF"); // less compact format
    TEST_ASSERT_CBOR2JSON("ui64", "4294967296", 0x6001, "1B 00 00 00 01 00 00 00 00");
    TEST_ASSERT_CBOR2JSON("ui64", "18446744073709551615", 0x6001, "1B FF FF FF FF FF FF FF FF");
#endif

    // int32 (positive values)
    TEST_ASSERT_CBOR2JSON("i32", "23", 0x6004, "17");
    TEST_ASSERT_CBOR2JSON("i32", "23", 0x6004, "18 17");       // less compact format
    TEST_ASSERT_CBOR2JSON("i32", "24", 0x6004, "18 18");
    TEST_ASSERT_CBOR2JSON("i32", "255", 0x6004, "18 ff");
    TEST_ASSERT_CBOR2JSON("i32", "255", 0x6004, "19 00 ff");   // less compact format
    TEST_ASSERT_CBOR2JSON("i32", "256", 0x6004, "19 01 00");
    TEST_ASSERT_CBOR2JSON("i32", "65535", 0x6004, "19 FF FF");
    TEST_ASSERT_CBOR2JSON("i32", "65535", 0x6004, "1A 00 00 FF FF");  // less compact format
    TEST_ASSERT_CBOR2JSON("i32", "65536", 0x6004, "1A 00 01 00 00");
    TEST_ASSERT_CBOR2JSON("i32", "2147483647", 0x6004, "1A 7F FF FF FF");      // maximum value for int32

#if TS_CONFIG_64BIT_TYPES_SUPPORT
    // int64 (positive values)
    TEST_ASSERT_CBOR2JSON("i64", "4294967295", 0x6002, "1A FF FF FF FF");
    TEST_ASSERT_CBOR2JSON("i64", "4294967296", 0x6002, "1B 00 00 00 01 00 00 00 00");
    TEST_ASSERT_CBOR2JSON("i64", "9223372036854775807", 0x6002, "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // int16 (negative values)
    TEST_ASSERT_CBOR2JSON("i16", "-24", 0x6006, "37");
    TEST_ASSERT_CBOR2JSON("i16", "-24", 0x6006, "38 17");      // less compact format
    TEST_ASSERT_CBOR2JSON("i16", "-25", 0x6006, "38 18");
    TEST_ASSERT_CBOR2JSON("i16", "-256", 0x6006, "38 ff");
    TEST_ASSERT_CBOR2JSON("i16", "-257", 0x6006, "39 01 00");
    TEST_ASSERT_CBOR2JSON("i16", "-32768", 0x6006, "39 7F FF");

    // int32 (negative values)
    TEST_ASSERT_CBOR2JSON("i32", "-24", 0x6004, "37");
    TEST_ASSERT_CBOR2JSON("i32", "-24", 0x6004, "38 17");      // less compact format
    TEST_ASSERT_CBOR2JSON("i32", "-25", 0x6004, "38 18");
    TEST_ASSERT_CBOR2JSON("i32", "-256", 0x6004, "38 ff");
    TEST_ASSERT_CBOR2JSON("i32", "-257", 0x6004, "39 01 00");
    TEST_ASSERT_CBOR2JSON("i32", "-65536", 0x6004, "39 FF FF");
    TEST_ASSERT_CBOR2JSON("i32", "-65537", 0x6004, "3A 00 01 00 00");
    TEST_ASSERT_CBOR2JSON("i32", "-2147483648", 0x6004, "3A 7F FF FF FF");      // maximum value for int32

#if TS_CONFIG_64BIT_TYPES_SUPPORT
    // int64 (negative values)
    TEST_ASSERT_CBOR2JSON("i64", "-4294967296", 0x6002, "3A FF FF FF FF");
    TEST_ASSERT_CBOR2JSON("i64", "-4294967297", 0x6002, "3B 00 00 00 01 00 00 00 00");
    TEST_ASSERT_CBOR2JSON("i64", "-9223372036854775808", 0x6002, "3B 7F FF FF FF FF FF FF FF"); // maximum value for int64
#endif

    // float
    TEST_ASSERT_CBOR2JSON("f32", "12.34",  0x6007, "fa 41 45 70 a4");
    TEST_ASSERT_CBOR2JSON("f32", "-12.34", 0x6007, "fa c1 45 70 a4");
    TEST_ASSERT_CBOR2JSON("f32", "12.34",  0x6007, "fa 41 45 81 06");      // 12.344
    TEST_ASSERT_CBOR2JSON("f32", "12.35",  0x6007, "fa 41 45 85 1f");      // 12.345 (should be rounded to 12.35)

#if TS_CONFIG_DECFRAC_TYPE_SUPPORT
    // decimal fraction
    TEST_ASSERT_CBOR2JSON("DecFrac_degC", "27315e-2", 0x600B, "c4 82 21 19 6a b3"); // decfrac 27315e-2
    TEST_ASSERT_CBOR2JSON("DecFrac_degC", "27315e-2", 0x600B, "c4 82 22 1a 00 04 2A FE"); // decfrac 273150e-3
    TEST_ASSERT_CBOR2JSON("DecFrac_degC", "27310e-2", 0x600B, "c4 82 20 19 0a ab"); // decfrac 2731e-1
    TEST_ASSERT_CBOR2JSON("DecFrac_degC", "27315e-2", 0x600B, "fa 43 88 93 33");    // float 273.15
    TEST_ASSERT_CBOR2JSON("DecFrac_degC", "27300e-2", 0x600B, "19 01 11");          // decimal 273
#endif

    // bool
    TEST_ASSERT_CBOR2JSON("bool", "true",  0x6008, "f5");
    TEST_ASSERT_CBOR2JSON("bool", "false",  0x6008, "f4");

    // string
    TEST_ASSERT_CBOR2JSON("strbuf", "\"Test\"",  0x6009, "64 54 65 73 74");
    TEST_ASSERT_CBOR2JSON("strbuf", "\"Hello World!\"",  0x6009, "6c 48 65 6c 6c 6f 20 57 6f 72 6c 64 21");
}

void tests_common(void)
{
    UNITY_BEGIN();

    // data conversion tests
    RUN_TEST(test_txt_patch_bin_fetch);
    RUN_TEST(test_bin_patch_txt_fetch);

    UNITY_END();
}
