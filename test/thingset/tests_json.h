
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
    TEST_ASSERT_EQUAL_STRING(":33 Unknown function.", resp_buf);
}

void json_write_wrong_data_structure()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"f32\":54.3");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":35 Wrong format.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf{\"f32\":54.3}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":35 Wrong format.", resp_buf);
}

void json_write_array()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {    \"f32\" : 52.8,\"i32\":50.6}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);
    TEST_ASSERT_EQUAL_FLOAT(52.8, f32);
    TEST_ASSERT_EQUAL(50, i32);
}

void json_write_readonly()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!info {\"i32_output\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":38 Unauthorized.", resp_buf);
}

void json_wrong_category()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!info {\"i32\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":42 Wrong category.", resp_buf);
}

void json_write_unknown()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"i3\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":34 Data object not found.", resp_buf);
}

void json_read_array()
{
    //                                                      float        bool         int
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"f32\",\"bool\",\"i32\"]");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [52.80,false,50]", resp_buf);
}

void json_read_rounded()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf \"f32_rounded\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success. 53", resp_buf);
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

void json_read_int32_array()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"arrayi32\"]");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [[4,2,8,4]]", resp_buf);
}

void json_read_float_array()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"arrayfloat\"]");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [[2.27,3.44]]", resp_buf);
}

void json_pub_msg()
{
    int resp_len = ts.pub_msg_json((char *)resp_buf, TS_RESP_BUFFER_LEN, 0);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING("# {\"ui32\":4294967295,\"i32\":50,\"ui16\":65535,\"i16\":-32768,\"f32\":52.80,\"bool\":false,\"strbuf\":\"Hello World!\"}", resp_buf);
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

void json_auth_user()
{
    ts.set_user_password("user123");

    // authorize as user
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"user123\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);

    // write user data
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_user\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);

    // attempt to write admin data
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_maker\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":38 Unauthorized.", resp_buf);
}

void json_auth_root()
{
    ts.set_maker_password("maker456");

    // authorize as root
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"maker456\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);

    // write user data
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_user\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);

    // write admin data
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_maker\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);
}

void json_auth_failure()
{
    ts.set_user_password("pass_user");

    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"abc\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":43 Wrong password.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_user\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":38 Unauthorized.", resp_buf);
}

void json_auth_long_password()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"012345678901234567890123456789\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":43 Wrong password.", resp_buf);

}

void json_auth_reset()
{
    ts.set_user_password("pass_user");

    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"pass_user\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_user\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":38 Unauthorized.", resp_buf);
}

void json_pub_list()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!pub");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success. [\"Serial_1s\"]", resp_buf);
}

void json_pub_enable()
{
    pub_channels[0].enabled = false;
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!pub {\"Serial_1s\":true}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":0 Success.", resp_buf);
    TEST_ASSERT_EQUAL(pub_channels[0].enabled, true);
}
