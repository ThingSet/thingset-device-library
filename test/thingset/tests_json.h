
#include "thingset.h"
#include "test_data.h"
#include "unity.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern ts_buffer_t req, resp;

void json_wrong_command()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!abcd \"f32\"");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":31 Unknown function.", resp.data.str);
}

void json_write_wrong_data_structure()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!write [\"f32\"]");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":33 Wrong format.", resp.data.str);

    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!write [\"f32\":54.3");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":33 Wrong format.", resp.data.str);

    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!write[\"f32\":54.3]");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":33 Wrong format.", resp.data.str);
}

void json_write_float()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!write \"f32\" : 54.3");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp.data.str);
    TEST_ASSERT_EQUAL_FLOAT(54.3, f32);
}

void json_write_int()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!write {\"i32\":61}");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp.data.str);
    TEST_ASSERT_EQUAL(61, i32);
}

void json_write_array()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!write {    \"f32\" : 52,\"i32\":50.6}");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp.data.str);
    TEST_ASSERT_EQUAL_FLOAT(52.0, f32);
    TEST_ASSERT_EQUAL(50, i32);
}

void json_write_readonly()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!write \"i32_output\" : 52");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":36 Unauthorized.", resp.data.str);
}

void json_write_unknown()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!write \"i3\" : 52");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":32 Data object not found.", resp.data.str);
}

void json_read_float()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!read \"f32\"");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":0 Success. 54.30", resp.data.str);
}

void json_read_array()
{
    //                                                      float        bool         int        
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!read [\"f32\", \"bool\", \"i32\"]");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [54.30, false, 61]", resp.data.str);
}

void json_list_input()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!list \"input\"");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [\"loadEnTarget\", \"usbEnTarget\"]", resp.data.str);

}

void json_list_all()
{
    req.pos = snprintf(req.data.str, TS_REQ_BUFFER_LEN, "!list ");
    thingset_process(&req, &resp, &data);
    TEST_ASSERT_EQUAL(strlen(resp.data.str), resp.pos);
    //TEST_ASSERT_EQUAL_STRING(":38 Response too long.", resp.data.str);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [\"manufacturer\", \"loadEnTarget\", \"usbEnTarget\", "
        "\"i32_output\", \"ui64\", \"i64\", \"ui32\", \"i32\", \"ui16\", \"i16\", \"f32\", \"bool\", \"strbuf\"]", resp.data.str);
}