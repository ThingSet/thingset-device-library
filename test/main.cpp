/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

#ifndef UNITY
#error "You have to include unity.h. Should be done by test.h->thingset_priv.h"
#endif


void setUp (void)
{
    (void)ts_init(&ts, &data_objects[0], data_objects_size);
}

void tearDown (void)
{
    // Nothing to do
}

void tests_common()
{
    UNITY_BEGIN();

    // data conversion tests
    RUN_TEST(test_txt_patch_bin_fetch);
    RUN_TEST(test_bin_patch_txt_fetch);

    UNITY_END();
}

void tests_text_mode()
{
    UNITY_BEGIN();

    // GET request
    RUN_TEST(test_txt_get_meas_names);
    RUN_TEST(test_txt_get_meas_names_values);
    RUN_TEST(test_txt_get_single_value);

    // FETCH request
    RUN_TEST(test_txt_fetch_array);
    RUN_TEST(test_txt_fetch_rounded);
    RUN_TEST(test_txt_fetch_nan);
    RUN_TEST(test_txt_fetch_inf);
    RUN_TEST(test_txt_fetch_int32_array);
    RUN_TEST(test_txt_fetch_float_array);

    // PATCH request
    RUN_TEST(test_txt_patch_wrong_data_structure);
    RUN_TEST(test_txt_patch_array);
    RUN_TEST(test_txt_patch_readonly);
    RUN_TEST(test_txt_patch_wrong_path);
    RUN_TEST(test_txt_patch_unknown_object);
    RUN_TEST(test_txt_conf_callback);

    // POST request
    RUN_TEST(test_txt_exec);

    // statements (pub/sub messages)
    RUN_TEST(test_txt_statement_subset);
    RUN_TEST(test_txt_statement_group);
    RUN_TEST(test_txt_pub_list_channels);
    RUN_TEST(test_txt_pub_enable);
    RUN_TEST(test_txt_pub_delete_append_object);

    // authentication
    RUN_TEST(test_txt_auth_user);
    RUN_TEST(test_txt_auth_root);
    RUN_TEST(test_txt_auth_long_password);
    RUN_TEST(test_txt_auth_failure);
    RUN_TEST(test_txt_auth_reset);

    // general tests
    RUN_TEST(test_txt_wrong_command);
    RUN_TEST(test_txt_get_endpoint);

    // data export
    RUN_TEST(test_txt_export);

    UNITY_END();
}

void tests_binary_mode()
{
    UNITY_BEGIN();

    // GET request
    RUN_TEST(test_bin_get_meas_ids_values);
    RUN_TEST(test_bin_get_meas_names_values);
    RUN_TEST(test_bin_get_single_value);

    // PATCH request
    RUN_TEST(test_bin_patch_multiple_objects);
    RUN_TEST(test_bin_patch_float_array);
    RUN_TEST(test_bin_patch_rounded_float);     // writes an integer to float

    // FETCH request
    RUN_TEST(test_bin_fetch_meas_ids);
    RUN_TEST(test_bin_fetch_meas_names);
    RUN_TEST(test_bin_fetch_multiple_objects);
    RUN_TEST(test_bin_fetch_float_array);
    RUN_TEST(test_bin_fetch_rounded_float);

    // POST request
    RUN_TEST(test_bin_exec);

    // pub/sub messages
    RUN_TEST(test_bin_statement_subset);
    RUN_TEST(test_bin_statement_group);
    RUN_TEST(test_bin_pub_can);

    // general tests
    RUN_TEST(test_bin_num_elem);
    RUN_TEST(test_bin_serialize_long_string);

    // binary (bytes) data type
#if TS_BYTE_STRING_TYPE_SUPPORT
    RUN_TEST(test_bin_serialize_bytes);
    RUN_TEST(test_bin_deserialize_bytes);
    RUN_TEST(test_bin_patch_fetch_bytes);
#endif

    // data export/import
    RUN_TEST(test_bin_export);
    RUN_TEST(test_bin_import);

    UNITY_END();
}

void tests_shim()
{
    UNITY_BEGIN();

    // data conversion tests
    RUN_TEST(test_shim_get_object);

    UNITY_END();
}

int main()
{
    tests_common();
    tests_text_mode();
    tests_binary_mode();
    tests_shim();
}
