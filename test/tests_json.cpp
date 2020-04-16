/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#include "tests.h"
#include "unity.h"

#include "thingset.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern uint8_t req_buf[];
extern uint8_t resp_buf[];
extern ThingSet ts;

extern float f32;
extern int32_t i32;
extern ArrayInfo int32_array;
extern ArrayInfo float32_array;
extern bool b;

extern bool pub_serial_enable;
extern ArrayInfo pub_serial_array;

void json_wrong_command()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!abcd \"f32\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A4 Not Found.", resp_buf);
}

void json_patch_wrong_data_structure()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"f32\":54.3");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A0 Bad Request.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf{\"f32\":54.3}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A4 Not Found.", resp_buf);
}

void json_patch_array()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {    \"f32\" : 52.8,\"i32\":50.6}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":84 Changed.", resp_buf);
    TEST_ASSERT_EQUAL_FLOAT(52.8, f32);
    TEST_ASSERT_EQUAL(50, i32);
}

void json_patch_readonly()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!test {\"i32_readonly\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A1 Unauthorized.", resp_buf);
}

void json_patch_wrong_path()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!info {\"i32\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A4 Not Found.", resp_buf);
}

void json_patch_unknown_node()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"i3\" : 52}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A4 Not Found.", resp_buf);
}

void json_fetch_array()
{
    b = false;
    //                                                      float        bool         int
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"f32\",\"bool\",\"i32\"]");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":85 Content. [52.80,false,50]", resp_buf);
}

void json_fetch_rounded()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf \"f32_rounded\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":85 Content. 53", resp_buf);
}

void json_list_output()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!output/");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":85 Content. [\"Bat_V\",\"Bat_A\",\"Ambient_degC\"]", resp_buf);
}

void json_get_output()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!output");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":85 Content. {\"Bat_V\":14.10,\"Bat_A\":5.13,\"Ambient_degC\":22}", resp_buf);
}

void json_fetch_int32_array()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"arrayi32\"]");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":85 Content. [[4,2,8,4]]", resp_buf);
}

void json_fetch_float_array()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf [\"arrayfloat\"]");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":85 Content. [[2.27,3.44]]", resp_buf);
}

void json_pub_msg()
{
    int resp_len = ts.pub_msg_json((char *)resp_buf, TS_RESP_BUFFER_LEN,
        (ts_node_id_t *)pub_serial_array.ptr, pub_serial_array.num_elements);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(
        "# {\"Timestamp_s\":12345678,\"Bat_V\":14.10,\"Bat_A\":5.13,\"Ambient_degC\":22}",
        resp_buf);
}

bool dummy_called_flag;

void dummy(void)
{
    dummy_called_flag = 1;
}

void json_exec()
{
    dummy_called_flag = 0;

    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!exec \"dummy\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":83 Valid.", resp_buf);
    TEST_ASSERT_EQUAL(1, dummy_called_flag);
}

bool conf_callback_called;

void conf_callback(void)        // implement function as defined in test_data.h
{
    conf_callback_called = 1;
}

void json_conf_callback()
{
    conf_callback_called = 0;
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"i32\":52}");

    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":84 Changed.", resp_buf);
    TEST_ASSERT_EQUAL(1, conf_callback_called);
}

void json_auth_user()
{
    ts.set_user_password("user123");

    // authorize as user
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"user123\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":83 Valid.", resp_buf);

    // write user data
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_user\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":84 Changed.", resp_buf);

    // attempt to write admin data
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_maker\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A1 Unauthorized.", resp_buf);
}

void json_auth_root()
{
    ts.set_maker_password("maker456");

    // authorize as root
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"maker456\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":83 Valid.", resp_buf);

    // write user data
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_user\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":84 Changed.", resp_buf);

    // write admin data
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_maker\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":84 Changed.", resp_buf);
}

void json_auth_failure()
{
    ts.set_user_password("pass_user");

    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"abc\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A9 Conflict.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_user\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A1 Unauthorized.", resp_buf);
}

void json_auth_long_password()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"012345678901234567890123456789\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A9 Conflict.", resp_buf);

}

void json_auth_reset()
{
    ts.set_user_password("pass_user");

    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth \"pass_user\"");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":83 Valid.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!auth");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":83 Valid.", resp_buf);

    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!conf {\"secret_user\" : 10}");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":A1 Unauthorized.", resp_buf);
}

void json_pub_list_channels()
{
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!pub/");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":85 Content. [\"serial\",\"can\"]", resp_buf);
}

void json_pub_enable()
{
    pub_serial_enable = false;
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "!pub/serial {\"Enable\":true}");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":84 Changed.", resp_buf);
    TEST_ASSERT_EQUAL(pub_serial_enable, true);
}

void json_get_endpoint_node()
{
    const DataNode *node;

    node = ts.get_endpoint_node("conf", strlen("conf"));
    TEST_ASSERT_NOT_NULL(node);
    TEST_ASSERT_EQUAL(node->id, TS_CONF);

    node = ts.get_endpoint_node("conf/", strlen("conf/"));
    TEST_ASSERT_NOT_NULL(node);
    TEST_ASSERT_EQUAL(node->id, TS_CONF);
}

void json_pub_delete_append_node()
{
    // before change
    size_t req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "?pub/serial/IDs");
    int resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(
        ":85 Content. [\"Timestamp_s\",\"Bat_V\",\"Bat_A\",\"Ambient_degC\"]", resp_buf);

    // delete "Ambient_degC"
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "-pub/serial/IDs \"Ambient_degC\"");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":82 Deleted.", resp_buf);

    // check if it was deleted
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "?pub/serial/IDs");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(
        ":85 Content. [\"Timestamp_s\",\"Bat_V\",\"Bat_A\"]", resp_buf);

    // append "Ambient_degC" again
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "+pub/serial/IDs \"Ambient_degC\"");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(":81 Created.", resp_buf);

    // check if it was appended
    req_len = snprintf((char *)req_buf, TS_REQ_BUFFER_LEN, "?pub/serial/IDs");
    resp_len = ts.process(req_buf, req_len, resp_buf, TS_RESP_BUFFER_LEN);
    TEST_ASSERT_EQUAL(strlen((char *)resp_buf), resp_len);
    TEST_ASSERT_EQUAL_STRING(
        ":85 Content. [\"Timestamp_s\",\"Bat_V\",\"Bat_A\",\"Ambient_degC\"]", resp_buf);
}

void tests_json()
{
    UNITY_BEGIN();

    RUN_TEST(json_wrong_command);
    RUN_TEST(json_patch_wrong_data_structure);
    RUN_TEST(json_patch_array);
    RUN_TEST(json_patch_readonly);
    RUN_TEST(json_patch_wrong_path);
    RUN_TEST(json_patch_unknown_node);
    RUN_TEST(json_fetch_array);
    RUN_TEST(json_fetch_rounded);
    RUN_TEST(json_list_output);
    RUN_TEST(json_get_output);
    RUN_TEST(json_exec);
    RUN_TEST(json_pub_msg);
    RUN_TEST(json_conf_callback);
    /* temporarily disabled
    RUN_TEST(json_auth_user);
    RUN_TEST(json_auth_root);
    RUN_TEST(json_auth_long_password);
    RUN_TEST(json_auth_failure);
    RUN_TEST(json_auth_reset);
    */
    RUN_TEST(json_pub_list_channels);
    RUN_TEST(json_pub_enable);
    RUN_TEST(json_pub_delete_append_node);
    RUN_TEST(json_get_endpoint_node);

    RUN_TEST(json_fetch_int32_array);
    RUN_TEST(json_fetch_float_array);

    UNITY_END();
}
