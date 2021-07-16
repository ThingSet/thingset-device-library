/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#if !CONFIG_THINGSET_ZEPHYR
#error "Compiling main.c in non Zephyr environment! You need to set CONFIG_THINGSET_ZEPHYR."
#endif

#include "../../../test/test.h"


void setup(void) {
    (void)ts_init(&ts, &data_nodes[0], data_nodes_size);
}

void teardown(void) {
}

void test_main(void)
{
    LOG_DBG("Running ThingSet tests for Zepyhr");

    ztest_test_suite(thingset_tests,
        /* test environment */
        ztest_unit_test(test_assert),
        ztest_unit_test(test_ts_init),
        /* data conversion tests */
        ztest_unit_test_setup_teardown(test_txt_patch_bin_fetch, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_patch_txt_fetch, setup, teardown),
        /* Text mode: GET request */
        ztest_unit_test_setup_teardown(test_txt_get_meas_names, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_get_meas_names_values, setup, teardown),
        /* Text mode: FETCH request */
        ztest_unit_test_setup_teardown(test_txt_fetch_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_rounded, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_nan, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_inf, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_int32_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_float_array, setup, teardown),
        /* Text mode: PATCH request */
        ztest_unit_test_setup_teardown(test_txt_patch_wrong_data_structure, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_patch_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_patch_readonly, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_patch_wrong_path, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_patch_unknown_node, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_conf_callback, setup, teardown),
        /* Text mode: POLL request */
        ztest_unit_test_setup_teardown(test_txt_exec, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_pub_msg, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_pub_list_channels, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_pub_enable, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_pub_delete_append_node, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_user, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_root, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_long_password, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_failure, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_reset, setup, teardown),
        /* Text mode: general tests */
        ztest_unit_test_setup_teardown(test_txt_wrong_command, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_get_endpoint, setup, teardown),
        /* Bin mode: GET request */
        ztest_unit_test_setup_teardown(test_bin_get_meas_ids, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_get_meas_names, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_get_meas_names_values, setup, teardown),
        /* Bin mode: PATCH request */
        ztest_unit_test_setup_teardown(test_bin_patch_multiple_nodes, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_patch_float_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_patch_rounded_float, setup, teardown),
        /* Text mode: FETCH request */
        ztest_unit_test_setup_teardown(test_bin_fetch_multiple_nodes, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_float_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_rounded_float, setup, teardown),
        /* Bin mode: POST request */
        ztest_unit_test_setup_teardown(test_bin_exec, setup, teardown),
        /* Bin mode: pub/sub messages */
        ztest_unit_test_setup_teardown(test_bin_pub, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_pub_can, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_sub, setup, teardown),
        /* Bin mode: general tests */
        ztest_unit_test_setup_teardown(test_bin_num_elem, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_serialize_long_string, setup, teardown),
        /* Bin mode: binary (bytes) data type */
        ztest_unit_test_setup_teardown(test_bin_serialize_bytes, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_deserialize_bytes, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_patch_fetch_bytes, setup, teardown)
    );

    ztest_run_test_suite(thingset_tests);
}
