/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <math.h>

#include "test.h"

void test_txt_get_root()
{
#if TS_NESTED_JSON
    TEST_ASSERT_TXT_REQ("?", ":85 Content. "
        "{\"t_s\":12345678,\"Info\":null,\"Conf\":null,\"Input\":null,\"Meas\":null,"
        "\"Nested\":null,\"Rec\":null,\"RPC\":null,\"Log\":2,"
        "\"mReport\":[\"t_s\",\"Meas/rBat_V\",\"Meas/rBat_A\",\"Meas/rAmbient_degC\"],"
        "\"_pub\":null,\"Test\":null}"
    );
#else
    TEST_ASSERT_TXT_REQ("?", ":85 Content. "
        "{\"t_s\":12345678,\"Info\":null,\"Conf\":null,\"Input\":null,\"Meas\":null,"
        "\"Nested\":null,\"Rec\":null,\"RPC\":null,\"Log\":2,"
        "\"mReport\":[\"t_s\",\"rBat_V\",\"rBat_A\",\"rAmbient_degC\"],"
        "\"_pub\":null,\"Test\":null}"
    );
#endif
}

void test_txt_get_meas_names()
{
    TEST_ASSERT_TXT_REQ("?Meas/", ":85 Content. [\"rBat_V\",\"rBat_A\",\"rAmbient_degC\"]");
}

void test_txt_get_meas_names_values()
{
    TEST_ASSERT_TXT_REQ("?Meas", ":85 Content. {\"rBat_V\":14.10,\"rBat_A\":5.13,\"rAmbient_degC\":22}");
}

void test_txt_get_single_value()
{
    TEST_ASSERT_TXT_REQ("?Meas/rBat_V", ":85 Content. 14.10");
}

void test_txt_get_nested()
{
    TEST_ASSERT_TXT_REQ("?Nested",
        ":85 Content. {\"Bat1\":null,\"rDummy_degC\":22,\"Bat2\":null,\"rAmbient_degC\":22}");
}

void test_txt_get_rpc()
{
    TEST_ASSERT_TXT_REQ("?RPC",
        ":85 Content. {\"xReset\":[],\"xAuth\":[\"uPassword\"],\"xDummy\":[]}");
}

void test_txt_fetch_array()
{
    f32 = 52.80;
    b = false;
    i32 = 50;

    TEST_ASSERT_TXT_REQ("?Conf [\"f32\",\"bool\",\"i32\"]", ":85 Content. [52.80,false,50]");
}

void test_txt_fetch_rounded()
{
    f32 = 52.80;

    TEST_ASSERT_TXT_REQ("?Conf \"f32_rounded\"", ":85 Content. 53");
}

void test_txt_fetch_nan()
{
    int nan = 0x7F800001;
    f32 = *(float*)&nan;

    TEST_ASSERT_TRUE(isnan(f32));

    TEST_ASSERT_TXT_REQ("?Conf \"f32\"", ":85 Content. null");
}

void test_txt_fetch_inf(void)
{
    int inf = 0x7F800000;
    f32 = *(float*)&inf;

    TEST_ASSERT_TXT_REQ("?Conf \"f32\"", ":85 Content. null");
}

void test_txt_fetch_int32_array(void)
{
    TEST_ASSERT_TXT_REQ("?Conf [\"arrayi32\"]", ":85 Content. [[4,2,8,4]]");
}

void test_txt_fetch_float_array(void)
{
    TEST_ASSERT_TXT_REQ("?Conf [\"arrayfloat\"]", ":85 Content. [[2.27,3.44]]");
}

void test_txt_fetch_num_records()
{
    TEST_ASSERT_TXT_REQ("?Log/", ":85 Content. 2");
    TEST_ASSERT_TXT_REQ("?Log", ":85 Content. 2");
}

void test_txt_fetch_record()
{
    TEST_ASSERT_TXT_REQ("?Log/1", ":85 Content. {\"t_s\":123,\"rBat_V\":14.50,\"sErrorFlags\":2}");
}

void test_txt_patch_wrong_data_structure(void)
{
    TEST_ASSERT_TXT_REQ("!Conf [\"f32\":54.3", ":A0 Bad Request.");
    TEST_ASSERT_TXT_REQ("!Conf{\"f32\":54.3}", ":A4 Not Found.");
}

void test_txt_patch_array(void)
{
    TEST_ASSERT_TXT_REQ("=Conf {    \"f32\" : 52.8,\"i32\":50.6}", ":84 Changed.");

    TEST_ASSERT_EQUAL_FLOAT(52.8, f32);
    TEST_ASSERT_EQUAL_INT32(50, i32);
}

void test_txt_patch_readonly(void)
{
    TEST_ASSERT_TXT_REQ("=Test {\"i32_readonly\" : 52}", ":A3 Forbidden.");
}

void test_txt_patch_wrong_path(void)
{
    TEST_ASSERT_TXT_REQ("=Info {\"i32\" : 52}", ":A4 Not Found.");
}

void test_txt_patch_unknown_object(void)
{
    TEST_ASSERT_TXT_REQ("=Conf {\"i3\" : 52}", ":A4 Not Found.");
}

void test_txt_group_callback(void)
{
    group_callback_called = false;

    TEST_ASSERT_TXT_REQ("=Conf {\"i32\":52}", ":84 Changed.");

    TEST_ASSERT_EQUAL(true, group_callback_called);
}

void test_txt_exec(void)
{
    dummy_called_flag = 0;

    TEST_ASSERT_TXT_REQ("!RPC/xDummy", ":83 Valid.");

    TEST_ASSERT_EQUAL(1, dummy_called_flag);
}

#if TS_NESTED_JSON

void test_txt_statement_subset(void)
{
    const char expected[] = "#mReport {\"t_s\":12345678,"
        "\"Meas\":{\"rBat_V\":14.10,\"rBat_A\":5.13,\"rAmbient_degC\":22}}";

    int resp_len = ts_txt_statement_by_path(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, "mReport");

    TEST_ASSERT_TXT_RESP(resp_len, expected);

    resp_len = ts_txt_statement_by_id(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, ID_REPORT);

    TEST_ASSERT_TXT_RESP(resp_len, expected);
}

#else

void test_txt_statement_subset(void)
{
    int resp_len = ts_txt_statement_by_path(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, "mReport");

    TEST_ASSERT_TXT_RESP(resp_len, "#mReport {\"t_s\":12345678,\"rBat_V\":14.10,\"rBat_A\":5.13,\"rAmbient_degC\":22}");

    resp_len = ts_txt_statement_by_id(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, ID_REPORT);

    TEST_ASSERT_TXT_RESP(resp_len, "#mReport {\"t_s\":12345678,\"rBat_V\":14.10,\"rBat_A\":5.13,\"rAmbient_degC\":22}");
}

#endif /* TS_NESTED_JSON */

void test_txt_statement_group(void)
{
    int resp_len = ts_txt_statement_by_path(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, "Info");

    TEST_ASSERT_TXT_RESP(resp_len, "#Info {\"cManufacturer\":\"Libre Solar\",\"cNodeID\":\"ABCD1234\"}");

    resp_len = ts_txt_statement_by_id(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, ID_INFO);

    TEST_ASSERT_TXT_RESP(resp_len, "#Info {\"cManufacturer\":\"Libre Solar\",\"cNodeID\":\"ABCD1234\"}");

    resp_len = ts_txt_statement_by_path(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, "Nested/Bat1");

    TEST_ASSERT_TXT_RESP(resp_len, "#Nested/Bat1 {\"r_V\":14.10,\"r_A\":5.13}");
}

void test_txt_statement_record(void)
{
    const char expected[] = "#Log/1 {\"t_s\":123,\"rBat_V\":14.50,\"sErrorFlags\":2}";

    struct ts_data_object *obj = ts_get_object_by_path(&ts, "Log", strlen("Log"));

    int resp_len = ts_txt_statement_record(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, obj, 1);

    TEST_ASSERT_TXT_RESP(resp_len, expected);
}

void test_txt_pub_list_channels(void)
{
    TEST_ASSERT_TXT_REQ("?_pub/", ":85 Content. [\"mReport\",\"Info\"]");
}

void test_txt_pub_enable(void)
{
    pub_report_enable = false;

    TEST_ASSERT_TXT_REQ("=_pub/mReport {\"wEnable\":true}", ":84 Changed.");

    TEST_ASSERT_TRUE(pub_report_enable);
}

#if TS_NESTED_JSON

void test_txt_pub_delete_append_object(void)
{
    /* before change */
    TEST_ASSERT_TXT_REQ("?mReport", ":85 Content. [\"t_s\",\"Meas/rBat_V\",\"Meas/rBat_A\",\"Meas/rAmbient_degC\"]");
    /* delete "rAmbient_degC" */
    TEST_ASSERT_TXT_REQ("-mReport \"Meas/rAmbient_degC\"", ":82 Deleted.");
    /* check if it was deleted */
    TEST_ASSERT_TXT_REQ("?mReport", ":85 Content. [\"t_s\",\"Meas/rBat_V\",\"Meas/rBat_A\"]");
    /* append "rAmbient_degC" again */
    TEST_ASSERT_TXT_REQ("+mReport \"Meas/rAmbient_degC\"", ":81 Created.");
    /* check if it was appended */
    TEST_ASSERT_TXT_REQ("?mReport", ":85 Content. [\"t_s\",\"Meas/rBat_V\",\"Meas/rBat_A\",\"Meas/rAmbient_degC\"]");
}

#else

void test_txt_pub_delete_append_object(void)
{
    /* before change */
    TEST_ASSERT_TXT_REQ("?mReport", ":85 Content. [\"t_s\",\"rBat_V\",\"rBat_A\",\"rAmbient_degC\"]");
    /* delete "rAmbient_degC" */
    TEST_ASSERT_TXT_REQ("-mReport \"rAmbient_degC\"", ":82 Deleted.");
    /* check if it was deleted */
    TEST_ASSERT_TXT_REQ("?mReport", ":85 Content. [\"t_s\",\"rBat_V\",\"rBat_A\"]");
    /* append "rAmbient_degC" again */
    TEST_ASSERT_TXT_REQ("+mReport \"rAmbient_degC\"", ":81 Created.");
    /* check if it was appended */
    TEST_ASSERT_TXT_REQ("?mReport", ":85 Content. [\"t_s\",\"rBat_V\",\"rBat_A\",\"rAmbient_degC\"]");
}

#endif /* TS_NESTED_JSON */

void test_txt_auth_user(void)
{
    /* authorize as expert user */
    TEST_ASSERT_TXT_REQ("!RPC/xAuth \"expert123\"", ":83 Valid.");
    /* write expert user data */
    TEST_ASSERT_TXT_REQ("=Conf {\"secret_expert\" : 10}", ":84 Changed.");
    /* attempt to write maker data */
    TEST_ASSERT_TXT_REQ("=Conf {\"secret_maker\" : 10}", ":A1 Unauthorized.");
}

void test_txt_auth_root(void)
{
    /* authorize as maker */
    TEST_ASSERT_TXT_REQ("!RPC/xAuth \"maker456\"", ":83 Valid.");
    /* write expert user data */
    TEST_ASSERT_TXT_REQ("=Conf {\"secret_expert\" : 10}", ":84 Changed.");
    /* write maker data */
    TEST_ASSERT_TXT_REQ("=Conf {\"secret_maker\" : 10}", ":84 Changed.");
}

void test_txt_auth_long_password(void)
{
    TEST_ASSERT_TXT_REQ("!RPC/xAuth \"012345678901234567890123456789\"", ":AF Unsupported Content-Format.");
}

void test_txt_auth_failure(void)
{
    TEST_ASSERT_TXT_REQ("!RPC/xAuth \"abc\"", ":83 Valid.");
    TEST_ASSERT_TXT_REQ("=Conf {\"secret_expert\" : 10}", ":A1 Unauthorized.");
}

void test_txt_auth_reset(void)
{
    TEST_ASSERT_TXT_REQ("!RPC/xAuth \"expert123\"", ":83 Valid.");
    TEST_ASSERT_TXT_REQ("!RPC/xAuth \"wrong\"", ":83 Valid.");
    TEST_ASSERT_TXT_REQ("=Conf {\"secret_expert\" : 10}", ":A1 Unauthorized.");
}

void test_txt_wrong_command(void)
{
    TEST_ASSERT_TXT_REQ("!abcd \"f32\"", ":A4 Not Found.");
}

void test_txt_get_endpoint(void)
{
    const struct ts_data_object *object;

    object = ts_get_object_by_path(&ts, "Conf", strlen("Conf"));
    TEST_ASSERT_NOT_NULL(object);
    TEST_ASSERT_EQUAL_UINT16(ID_CONF, object->id);

    object = ts_get_object_by_path(&ts, "Conf/", strlen("Conf/"));
    TEST_ASSERT_NOT_NULL(object);
    TEST_ASSERT_EQUAL_UINT16(ID_CONF, object->id);

    object = ts_get_object_by_path(&ts, "/", strlen("/"));
    TEST_ASSERT_NULL(object);

    /* special case where the data contains forward slashes */
    object = ts_get_object_by_path(&ts, "Conf \"this/is/a/path\"", strlen("Conf"));
    TEST_ASSERT_NOT_NULL(object);
    TEST_ASSERT_EQUAL_UINT16(ID_CONF, object->id);

    /* special case where the data contains forward slashes */
    object = ts_get_object_by_path(&ts, "RPC/xReset \"this/is/a/path\"", strlen("RPC/xReset"));
    TEST_ASSERT_NOT_NULL(object);
    TEST_ASSERT_EQUAL_UINT16(0xE1, object->id);
}

#if TS_NESTED_JSON

void test_txt_export(void)
{
    char *expected =
        "{\"t_s\":12345678,\"Meas\":{\"rBat_V\":14.10,\"rBat_A\":5.13,\"rAmbient_degC\":22}}";

    int resp_len = ts_txt_export(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, SUBSET_REPORT);
    resp_buf[resp_len] = '\0';

    TEST_ASSERT_TXT_RESP(resp_len, expected);

    expected =
        "{\"Nested\":{"
            "\"Bat1\":{\"r_V\":14.10,\"r_A\":5.13},"
            "\"Bat2\":{\"r_V\":14.10,\"r_A\":5.13},"
            "\"rAmbient_degC\":22}"
        "}";

    resp_len = ts_txt_export(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, SUBSET_NESTED);
    resp_buf[resp_len] = '\0';

    TEST_ASSERT_TXT_RESP(resp_len, expected);
}

#else

void test_txt_export(void)
{
    int resp_len = ts_txt_export(&ts, (char *)resp_buf, TS_RESP_BUFFER_LEN, SUBSET_REPORT);

    TEST_ASSERT_TXT_RESP(resp_len, "{\"t_s\":12345678,\"rBat_V\":14.10,\"rBat_A\":5.13,\"rAmbient_degC\":22}");
}

#endif /* TS_NESTED_JSON */

void test_txt_update_callback(void)
{
    update_callback_called = false;

    // without callback
    ts_set_update_callback(&ts, SUBSET_NVM, NULL);
    TEST_ASSERT_TXT_REQ("=Conf {\"sBatCharging_V\":52}", ":84 Changed.");
    TEST_ASSERT_EQUAL(false, update_callback_called);

    // with configured callback
    ts_set_update_callback(&ts, SUBSET_NVM, update_callback);
    TEST_ASSERT_TXT_REQ("=Conf {\"sBatCharging_V\":52}", ":84 Changed.");
    TEST_ASSERT_EQUAL(true, update_callback_called);
}
