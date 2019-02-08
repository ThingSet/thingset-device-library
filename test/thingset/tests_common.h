
#include "thingset.h"
#include "cbor.h"
#include "test_data.h"
#include "unity.h"
#include <inttypes.h>

extern uint8_t req_buf[];
extern uint8_t resp_buf[];
extern ThingSet ts;

//extern ts_buffer_t req, resp;

void _write_json(char const *name, char const *value)
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"%s\":%s}", name, value);
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);
}

int _read_json(char const *name, char *value_read)
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf \"%s\"", name);
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);

    char buf[100];
    strncpy(buf, (char *)resp_buf, 11);
    buf[11] = '\0';
    //printf("buf: %s, resp: %s\n", buf, resp_buf);

    TEST_ASSERT_EQUAL_STRING(":0 Success.", buf);

    return snprintf(value_read, strlen((char *)resp_buf) - 11, "%s", resp_buf + 12);
}

// returns length of read value
int _read_cbor(uint16_t id, char *value_read)
{
    // generate read request
    req_buf[0] = TS_FUNCTION_CONF;
    req_buf[1] = 0x19;     // uint16 follows
    req_buf[2] = id >> 8;
    req_buf[3] = (uint8_t)id;
    size_t req_len = 4;
    ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    //printf("TEST: Read request len: %d, response len: %d, resp code:%d\n", req_len, resp_len, resp_buf[0]);
    TEST_ASSERT_EQUAL_UINT8(TS_STATUS_SUCCESS, resp_buf[0] - 0x80);
    //printf("TEST: Read request len: %d, response len: %d\n", req.pos, resp_len);

    int value_len = cbor_size((uint8_t*)resp_buf + 1);
    memcpy(value_read, resp_buf + 1, value_len);
    return value_len;
}

// returns length of read value
void _write_cbor(uint16_t id, char *value)
{
    int len = cbor_size((uint8_t*)value);

    // generate write request
    req_buf[0] = TS_FUNCTION_WRITE;
    req_buf[1] = 0xA1;     // map with 1 element
    req_buf[2] = 0x19;     // uint16 follows
    req_buf[3] = id >> 8;
    req_buf[4] = (uint8_t)id;
    memcpy(req_buf + 5, value, len);
    size_t req_len = len + 5;
    ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL_UINT8(TS_STATUS_SUCCESS, resp_buf[0] - 0x80);
}

void _json2cbor(char const *name, char const *json_value, uint16_t id, char const *cbor_value_hex)
{
    char buf[100];  // temporary data storage (JSON or CBOR)

    // extract binary CBOR data
    char cbor_value[100];
    int len = strlen(cbor_value_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_value[pos++] = (char)strtoul(&cbor_value_hex[i], NULL, 16);
    }

    //printf ("json2cbor(\"%s\", \"%s\", 0x%x, 0x(%s) )\n", name, json_value, id, cbor_value_hex);

    //printf("before write: i64 = 0x%" PRIi64 " = %" PRIx64 "\n", i64, i64);
    //printf("before write: i64 = %16.llX = %lli\n", i64, i64);
    _write_json(name, json_value);
    //printf("after write:  i64 = %16.llX = %lli\n", i64, i64);
    //printf("after write: i64 = %" PRIi64 " = %llx\n", i64, i64);
    len = _read_cbor(id, buf);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(cbor_value, buf, len);
}

void _cbor2json(char const *name, char const *json_value, uint16_t id, char const *cbor_value_hex)
{
    char buf[100];  // temporary data storage (JSON or CBOR)

    // extract binary CBOR data
    char cbor_value[100];
    int len = strlen(cbor_value_hex);
    int pos = 0;
    for (int i = 0; i < len; i += 3) {
        cbor_value[pos++] = (char)strtoul(&cbor_value_hex[i], NULL, 16);
    }

    //printf ("cbor2json(\"%s\", \"%s\", 0x%x, 0x(%s) )\n", name, json_value, id, cbor_value_hex);

    _write_cbor(id, cbor_value);
    len = _read_json(name, buf);

    TEST_ASSERT_EQUAL_STRING(json_value, buf);
}

void write_json_read_cbor()
{
    // uint16
    _json2cbor("ui16", "0", 0x6005, "00");
    _json2cbor("ui16", "23", 0x6005, "17");
    _json2cbor("ui16", "24", 0x6005, "18 18");
    _json2cbor("ui16", "255", 0x6005, "18 ff");
    _json2cbor("ui16", "256", 0x6005, "19 01 00");
    _json2cbor("ui16", "65535", 0x6005, "19 FF FF");

    // uint32
    _json2cbor("ui32", "0", 0x6003, "00");
    _json2cbor("ui32", "23", 0x6003, "17");
    _json2cbor("ui32", "24", 0x6003, "18 18");
    _json2cbor("ui32", "255", 0x6003, "18 ff");
    _json2cbor("ui32", "256", 0x6003, "19 01 00");
    _json2cbor("ui32", "65535", 0x6003, "19 FF FF");
    _json2cbor("ui32", "65536", 0x6003, "1A 00 01 00 00");
    _json2cbor("ui32", "4294967295", 0x6003, "1A FF FF FF FF");

    // uint64
    #if TS_64BIT_TYPES_SUPPORT
    _json2cbor("ui64", "4294967295", 0x6001, "1A FF FF FF FF");
    _json2cbor("ui64", "4294967296", 0x6001, "1B 00 00 00 01 00 00 00 00");
    _json2cbor("ui64", "9223372036854775807", 0x6001, "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
    #endif

    // int16 (positive values)
    _json2cbor("i16", "0", 0x6006, "00");
    _json2cbor("i16", "23", 0x6006, "17");
    _json2cbor("i16", "24", 0x6006, "18 18");
    _json2cbor("i16", "255", 0x6006, "18 ff");
    _json2cbor("i16", "256", 0x6006, "19 01 00");
    _json2cbor("i16", "32767", 0x6006, "19 7F FF");                 // maximum value for int16

    // int32 (positive values)
    _json2cbor("i32", "0", 0x6004, "00");
    _json2cbor("i32", "23", 0x6004, "17");
    _json2cbor("i32", "24", 0x6004, "18 18");
    _json2cbor("i32", "255", 0x6004, "18 ff");
    _json2cbor("i32", "256", 0x6004, "19 01 00");
    _json2cbor("i32", "65535", 0x6004, "19 FF FF");
    _json2cbor("i32", "65536", 0x6004, "1A 00 01 00 00");
    _json2cbor("i32", "2147483647", 0x6004, "1A 7F FF FF FF");      // maximum value for int32

    // int64 (positive values)
    #if TS_64BIT_TYPES_SUPPORT
    _json2cbor("i64", "4294967295", 0x6002, "1A FF FF FF FF");
    _json2cbor("i64", "4294967296", 0x6002, "1B 00 00 00 01 00 00 00 00");
    _json2cbor("i64", "9223372036854775807", 0x6002, "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
    #endif

    // int16 (negative values)
    _json2cbor("i16", "-0", 0x6006, "00");
    _json2cbor("i16", "-24", 0x6006, "37");
    _json2cbor("i16", "-25", 0x6006, "38 18");
    _json2cbor("i16", "-256", 0x6006, "38 ff");
    _json2cbor("i16", "-257", 0x6006, "39 01 00");
    _json2cbor("i16", "-32768", 0x6006, "39 7F FF");

    // int32 (negative values)
    _json2cbor("i32", "-0", 0x6004, "00");
    _json2cbor("i32", "-24", 0x6004, "37");
    _json2cbor("i32", "-25", 0x6004, "38 18");
    _json2cbor("i32", "-256", 0x6004, "38 ff");
    _json2cbor("i32", "-257", 0x6004, "39 01 00");
    _json2cbor("i32", "-65536", 0x6004, "39 FF FF");
    _json2cbor("i32", "-65537", 0x6004, "3A 00 01 00 00");
    _json2cbor("i32", "-2147483648", 0x6004, "3A 7F FF FF FF");      // maximum value for int32

    // int64 (negative values)
    #if TS_64BIT_TYPES_SUPPORT
    _json2cbor("i64", "-4294967296", 0x6002, "3A FF FF FF FF");
    _json2cbor("i64", "-4294967297", 0x6002, "3B 00 00 00 01 00 00 00 00");
    _json2cbor("i64", "-9223372036854775808", 0x6002, "3B 7F FF FF FF FF FF FF FF"); // maximum value for int64
    #endif

    // float
    _json2cbor("f32", "12.340",  0x6007, "fa 41 45 70 a4");
    _json2cbor("f32", "-12.340", 0x6007, "fa c1 45 70 a4");
    _json2cbor("f32", "12.345",  0x6007, "fa 41 45 85 1f");

    // bool
    _json2cbor("bool", "true",  0x6008, "f5");
    _json2cbor("bool", "false",  0x6008, "f4");

    // string
    _json2cbor("strbuf", "\"Test\"",  0x6009, "64 54 65 73 74");
    _json2cbor("strbuf", "\"Hello World!\"",  0x6009, "6c 48 65 6c 6c 6f 20 57 6f 72 6c 64 21");
}


void write_cbor_read_json()
{
    // uint16
    _cbor2json("ui16", "0", 0x6005, "00");
    _cbor2json("ui16", "23", 0x6005, "17");
    _cbor2json("ui16", "23", 0x6005, "18 17");       // less compact format
    _cbor2json("ui16", "24", 0x6005, "18 18");
    _cbor2json("ui16", "255", 0x6005, "18 ff");
    _cbor2json("ui16", "255", 0x6005, "19 00 ff");   // less compact format
    _cbor2json("ui16", "256", 0x6005, "19 01 00");
    _cbor2json("ui16", "65535", 0x6005, "19 FF FF");

    // uint32
    _cbor2json("ui32", "0", 0x6003, "00");
    _cbor2json("ui32", "23", 0x6003, "17");
    _cbor2json("ui32", "23", 0x6003, "18 17");       // less compact format
    _cbor2json("ui32", "24", 0x6003, "18 18");
    _cbor2json("ui32", "255", 0x6003, "18 ff");
    _cbor2json("ui32", "255", 0x6003, "19 00 ff");   // less compact format
    _cbor2json("ui32", "256", 0x6003, "19 01 00");
    _cbor2json("ui32", "65535", 0x6003, "19 FF FF");
    _cbor2json("ui32", "65535", 0x6003, "1A 00 00 FF FF");  // less compact format
    _cbor2json("ui32", "65536", 0x6003, "1A 00 01 00 00");
    _json2cbor("ui32", "4294967295", 0x6003, "1A FF FF FF FF");

    // uint64
    #if TS_64BIT_TYPES_SUPPORT
    _cbor2json("ui64", "4294967295", 0x6001, "1A FF FF FF FF");
    _cbor2json("ui64", "4294967295", 0x6001, "1B 00 00 00 00 FF FF FF FF"); // less compact format
    _cbor2json("ui64", "4294967296", 0x6001, "1B 00 00 00 01 00 00 00 00");
    _cbor2json("ui64", "18446744073709551615", 0x6001, "1B FF FF FF FF FF FF FF FF");
    #endif

    // int32 (positive values)
    _cbor2json("i32", "23", 0x6004, "17");
    _cbor2json("i32", "23", 0x6004, "18 17");       // less compact format
    _cbor2json("i32", "24", 0x6004, "18 18");
    _cbor2json("i32", "255", 0x6004, "18 ff");
    _cbor2json("i32", "255", 0x6004, "19 00 ff");   // less compact format
    _cbor2json("i32", "256", 0x6004, "19 01 00");
    _cbor2json("i32", "65535", 0x6004, "19 FF FF");
    _cbor2json("i32", "65535", 0x6004, "1A 00 00 FF FF");  // less compact format
    _cbor2json("i32", "65536", 0x6004, "1A 00 01 00 00");
    _cbor2json("i32", "2147483647", 0x6004, "1A 7F FF FF FF");      // maximum value for int32

    // int64 (positive values)
    #if TS_64BIT_TYPES_SUPPORT
    _cbor2json("i64", "4294967295", 0x6002, "1A FF FF FF FF");
    _cbor2json("i64", "4294967296", 0x6002, "1B 00 00 00 01 00 00 00 00");
    _cbor2json("i64", "9223372036854775807", 0x6002, "1B 7F FF FF FF FF FF FF FF"); // maximum value for int64
    #endif

    // int16 (negative values)
    _cbor2json("i16", "-24", 0x6006, "37");
    _cbor2json("i16", "-24", 0x6006, "38 17");      // less compact format
    _cbor2json("i16", "-25", 0x6006, "38 18");
    _cbor2json("i16", "-256", 0x6006, "38 ff");
    _cbor2json("i16", "-257", 0x6006, "39 01 00");
    _cbor2json("i16", "-32768", 0x6006, "39 7F FF");

    // int32 (negative values)
    _cbor2json("i32", "-24", 0x6004, "37");
    _cbor2json("i32", "-24", 0x6004, "38 17");      // less compact format
    _cbor2json("i32", "-25", 0x6004, "38 18");
    _cbor2json("i32", "-256", 0x6004, "38 ff");
    _cbor2json("i32", "-257", 0x6004, "39 01 00");
    _cbor2json("i32", "-65536", 0x6004, "39 FF FF");
    _cbor2json("i32", "-65537", 0x6004, "3A 00 01 00 00");
    _cbor2json("i32", "-2147483648", 0x6004, "3A 7F FF FF FF");      // maximum value for int32

    // int64 (negative values)
    #if TS_64BIT_TYPES_SUPPORT
    _cbor2json("i64", "-4294967296", 0x6002, "3A FF FF FF FF");
    _cbor2json("i64", "-4294967297", 0x6002, "3B 00 00 00 01 00 00 00 00");
    _cbor2json("i64", "-9223372036854775808", 0x6002, "3B 7F FF FF FF FF FF FF FF"); // maximum value for int64
    #endif

    // float
    _cbor2json("f32", "12.34",  0x6007, "fa 41 45 70 a4");
    _cbor2json("f32", "-12.34", 0x6007, "fa c1 45 70 a4");
    _cbor2json("f32", "12.34",  0x6007, "fa 41 45 81 06");      // 12.344
    _cbor2json("f32", "12.35",  0x6007, "fa 41 45 85 1f");      // 12.345 (should be rounded to 12.35)

    // bool
    _cbor2json("bool", "true",  0x6008, "f5");
    _cbor2json("bool", "false",  0x6008, "f4");

    // string
    _cbor2json("strbuf", "\"Test\"",  0x6009, "64 54 65 73 74");
    _cbor2json("strbuf", "\"Hello World!\"",  0x6009, "6c 48 65 6c 6c 6f 20 57 6f 72 6c 64 21");
}