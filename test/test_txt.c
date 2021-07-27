/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>

#include "test.h"

void test_txt_get_output_names()
{
    TEST_ASSERT_TXT_REQ("?output/", ":85 Content. [\"Bat_V\",\"Bat_A\",\"Ambient_degC\"]");
}

void test_txt_get_output_names_values()
{
    TEST_ASSERT_TXT_REQ("?output", ":85 Content. {\"Bat_V\":14.10,\"Bat_A\":5.13,\"Ambient_degC\":22}");
}

void test_txt_fetch_array()
{
    f32 = 52.80;
    b = false;
    i32 = 50;

    TEST_ASSERT_TXT_REQ("?conf [\"f32\",\"bool\",\"i32\"]", ":85 Content. [52.80,false,50]");
}

void test_txt_fetch_rounded()
{
    f32 = 52.80;

    TEST_ASSERT_TXT_REQ("?conf \"f32_rounded\"", ":85 Content. 53");
}

void test_txt_fetch_nan()
{
    int nan = 0x7F800001;
    f32 = *(float*)&nan;

    TEST_ASSERT_TRUE(isnan(f32));

    TEST_ASSERT_TXT_REQ("?conf \"f32\"", ":85 Content. null");
}

void test_txt_fetch_inf(void)
{
    int inf = 0x7F800000;
    f32 = *(float*)&inf;

    TEST_ASSERT_TXT_REQ("?conf \"f32\"", ":85 Content. null");
}

void test_txt_fetch_int32_array(void)
{
    TEST_ASSERT_TXT_REQ("?conf [\"arrayi32\"]", ":85 Content. [[4,2,8,4]]");
}

void test_txt_fetch_float_array(void)
{
    TEST_ASSERT_TXT_REQ("?conf [\"arrayfloat\"]", ":85 Content. [[2.27,3.44]]");
}

void test_txt_patch_wrong_data_structure(void)
{
    TEST_ASSERT_TXT_REQ("!conf [\"f32\":54.3", ":A0 Bad Request.");
    TEST_ASSERT_TXT_REQ("!conf{\"f32\":54.3}", ":A4 Not Found.");
}

void test_txt_patch_array(void)
{
    TEST_ASSERT_TXT_REQ("=conf {    \"f32\" : 52.8,\"i32\":50.6}", ":84 Changed.");

    TEST_ASSERT_EQUAL_FLOAT(52.8, f32);
    TEST_ASSERT_EQUAL_INT32(50, i32);
}

void test_txt_patch_readonly(void)
{
    TEST_ASSERT_TXT_REQ("=test {\"i32_readonly\" : 52}", ":A3 Forbidden.");
}

void test_txt_patch_wrong_path(void)
{
    TEST_ASSERT_TXT_REQ("=info {\"i32\" : 52}", ":A4 Not Found.");
}

void test_txt_patch_unknown_node(void)
{
    TEST_ASSERT_TXT_REQ("=conf {\"i3\" : 52}", ":A4 Not Found.");
}

void test_txt_conf_callback(void)
{
    conf_callback_called = 0;

    TEST_ASSERT_TXT_REQ("=conf {\"i32\":52}", ":84 Changed.");

    TEST_ASSERT_EQUAL(1, conf_callback_called);
}

void test_txt_exec(void)
{
    dummy_called_flag = 0;

    TEST_ASSERT_TXT_REQ("!exec/dummy", ":83 Valid.");

    TEST_ASSERT_EQUAL(1, dummy_called_flag);
}

void test_txt_pub_msg(void)
{
    int resp_len = ts_txt_pub(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, PUB_SER);

    TEST_ASSERT_TXT_RESP(resp_len, "# {\"Timestamp_s\":12345678,\"Bat_V\":14.10,\"Bat_A\":5.13,\"Ambient_degC\":22}");
}

void test_txt_pub_list_channels(void)
{
    TEST_ASSERT_TXT_REQ("?pub/", ":85 Content. [\"serial\",\"can\"]");
}

void test_txt_pub_enable(void)
{
    pub_serial_enable = false;

    TEST_ASSERT_TXT_REQ("=pub/serial {\"Enable\":true}", ":84 Changed.");

    TEST_ASSERT_TRUE(pub_serial_enable);
}

void test_txt_pub_delete_append_node(void)
{
    /* before change */
    TEST_ASSERT_TXT_REQ("?pub/serial/IDs", ":85 Content. [\"Timestamp_s\",\"Bat_V\",\"Bat_A\",\"Ambient_degC\"]");
    /* delete "Ambient_degC" */
    TEST_ASSERT_TXT_REQ("-pub/serial/IDs \"Ambient_degC\"", ":82 Deleted.");
    /* check if it was deleted */
    TEST_ASSERT_TXT_REQ("?pub/serial/IDs", ":85 Content. [\"Timestamp_s\",\"Bat_V\",\"Bat_A\"]");
    /* append "Ambient_degC" again */
    TEST_ASSERT_TXT_REQ("+pub/serial/IDs \"Ambient_degC\"", ":81 Created.");
    /* check if it was appended */
    TEST_ASSERT_TXT_REQ("?pub/serial/IDs", ":85 Content. [\"Timestamp_s\",\"Bat_V\",\"Bat_A\",\"Ambient_degC\"]");
}

void test_txt_auth_user(void)
{
    /* authorize as expert user */
    TEST_ASSERT_TXT_REQ("!auth \"expert123\"", ":83 Valid.");
    /* write expert user data */
    TEST_ASSERT_TXT_REQ("=conf {\"secret_expert\" : 10}", ":84 Changed.");
    /* attempt to write maker data */
    TEST_ASSERT_TXT_REQ("=conf {\"secret_maker\" : 10}", ":A1 Unauthorized.");
}

void test_txt_auth_root(void)
{
    /* authorize as maker */
    TEST_ASSERT_TXT_REQ("!auth \"maker456\"", ":83 Valid.");
    /* write expert user data */
    TEST_ASSERT_TXT_REQ("=conf {\"secret_expert\" : 10}", ":84 Changed.");
    /* write maker data */
    TEST_ASSERT_TXT_REQ("=conf {\"secret_maker\" : 10}", ":84 Changed.");
}

void test_txt_auth_long_password(void)
{
    TEST_ASSERT_TXT_REQ("!auth \"012345678901234567890123456789\"", ":AF Unsupported Content-Format.");
}

void test_txt_auth_failure(void)
{
    TEST_ASSERT_TXT_REQ("!auth \"abc\"", ":83 Valid.");
    TEST_ASSERT_TXT_REQ("=conf {\"secret_expert\" : 10}", ":A1 Unauthorized.");
}

void test_txt_auth_reset(void)
{
    TEST_ASSERT_TXT_REQ("!auth \"expert123\"", ":83 Valid.");
    TEST_ASSERT_TXT_REQ("!auth \"wrong\"", ":83 Valid.");
    TEST_ASSERT_TXT_REQ("=conf {\"secret_expert\" : 10}", ":A1 Unauthorized.");
}

void test_txt_wrong_command(void)
{
    TEST_ASSERT_TXT_REQ("!abcd \"f32\"", ":A4 Not Found.");
}

void test_txt_get_endpoint(void)
{
    const struct ts_data_node *node;

    node = ts_get_node_by_path(&ts, "conf", strlen("conf"));
    TEST_ASSERT_NOT_NULL(node);
    TEST_ASSERT_EQUAL_UINT16(ID_CONF, node->id);

    node = ts_get_node_by_path(&ts, "conf/", strlen("conf/"));
    TEST_ASSERT_NOT_NULL(node);
    TEST_ASSERT_EQUAL_UINT16(ID_CONF, node->id);

    node = ts_get_node_by_path(&ts, "/", strlen("/"));
    TEST_ASSERT_NULL(node);

    /* special case where the data contains forward slashes */
    node = ts_get_node_by_path(&ts, "conf \"this/is/a/path\"", strlen("conf"));
    TEST_ASSERT_NOT_NULL(node);
    TEST_ASSERT_EQUAL_UINT16(ID_CONF, node->id);

    /* special case where the data contains forward slashes */
    node = ts_get_node_by_path(&ts, "exec/reset \"this/is/a/path\"", strlen("exec/reset"));
    TEST_ASSERT_NOT_NULL(node);
    TEST_ASSERT_EQUAL_UINT16(0xE1, node->id);
}
