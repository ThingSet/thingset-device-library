
#include "thingset.h"
#include "test_data.h"
#include "unity.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t req_buf[];
extern uint8_t resp_buf[];
extern ThingSet ts;

void json_wrong_command()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!abcd \"f32\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":31 Unknown function.", resp_buf);
}

void json_write_wrong_data_structure()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"f32\":54.3");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":33 Wrong format.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf{\"f32\":54.3}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":33 Wrong format.", resp_buf);
}

void json_write_array()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {    \"f32\" : 52,\"i32\":50.6}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);
    TEST_ASSERT_EQUAL_FLOAT(52.0, f32);
    TEST_ASSERT_EQUAL(50, i32);
}

void json_write_readonly()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!info {\"i32_output\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":36 Unauthorized.", resp_buf);
}

void json_wrong_category()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!info {\"i32\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":40 Wrong category.", resp_buf);
}

void json_write_unknown()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"i3\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":32 Data object not found.", resp_buf);
}

void json_read_array()
{
    //                                                      float        bool         int
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"f32\",\"bool\",\"i32\"]");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [52.00,false,50]", resp_buf);
}

void json_list_input()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!input");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [\"loadEnTarget\",\"usbEnTarget\"]", resp_buf);
}

void json_list_names_values_input()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!input {}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success. {\"loadEnTarget\":false,\"usbEnTarget\":false}", resp_buf);
}

void json_pub_msg()
{
    int resp_len = ts.pub_msg_json((char *)resp_buf, TS_RESP_BUFFER_LEN, 0);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING("# {\"ui32\":4294967295,\"i32\":50,\"ui16\":65535,\"i16\":-32768,\"f32\":52.00,\"bool\":false,\"strbuf\":\"Hello World!\"}", resp_buf);
}

extern bool dummy_called_flag;

void json_exec()
{
    dummy_called_flag = 0;

    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!exec \"dummy\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);
    TEST_ASSERT_EQUAL(1, dummy_called_flag);
}

void json_conf_callback()
{
    dummy_called_flag = 0;
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"i32\":52}");

    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);
    TEST_ASSERT_EQUAL(0, dummy_called_flag);

    ts.set_conf_callback(dummy);

    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);
    TEST_ASSERT_EQUAL(1, dummy_called_flag);
}