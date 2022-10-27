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
    (void)ts_init(&ts, &data_objects[0], data_objects_size);
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
        ztest_unit_test_setup_teardown(test_txt_get_root, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_get_meas_names, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_get_meas_names_values, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_get_single_value, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_get_nested, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_get_rpc, setup, teardown),
        /* Text mode: FETCH request */
        ztest_unit_test_setup_teardown(test_txt_fetch_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_rounded, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_nan, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_inf, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_int32_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_float_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_num_records, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_fetch_record, setup, teardown),
        /* Text mode: PATCH request */
        ztest_unit_test_setup_teardown(test_txt_patch_wrong_data_structure, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_patch_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_patch_readonly, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_patch_wrong_path, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_patch_unknown_object, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_group_callback, setup, teardown),
        /* Text mode: POST request */
        ztest_unit_test_setup_teardown(test_txt_exec, setup, teardown),
        /* Text mode: statements (pub/sub messages) */
        ztest_unit_test_setup_teardown(test_txt_statement_subset, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_statement_group, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_statement_record, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_pub_list_channels, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_pub_enable, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_pub_delete_append_object, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_user, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_root, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_long_password, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_failure, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_auth_reset, setup, teardown),
        /* Text mode: general tests */
        ztest_unit_test_setup_teardown(test_txt_wrong_command, setup, teardown),
        ztest_unit_test_setup_teardown(test_txt_get_endpoint, setup, teardown),
        /* Text mode: exporting of data */
        ztest_unit_test_setup_teardown(test_txt_export, setup, teardown),
        /* Text mode: update notification */
        ztest_unit_test_setup_teardown(test_txt_update_callback, setup, teardown),

        /* Bin mode: GET request */
        ztest_unit_test_setup_teardown(test_bin_get_meas_ids_values, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_get_meas_names_values, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_get_single_value, setup, teardown),
        /* Bin mode: PATCH request */
        ztest_unit_test_setup_teardown(test_bin_patch_multiple_objects, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_patch_float_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_patch_rounded_float, setup, teardown),
        /* Text mode: FETCH request */
        ztest_unit_test_setup_teardown(test_bin_fetch_meas_ids, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_meas_names, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_multiple_objects, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_float_array, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_rounded_float, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_by_name, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_num_records, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_record, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_record_item, setup, teardown),
        /* Bin mode: POST request */
        ztest_unit_test_setup_teardown(test_bin_exec, setup, teardown),
        /* Bin mode: pub/sub messages */
        ztest_unit_test_setup_teardown(test_bin_statement_subset, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_statement_group, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_pub_can, setup, teardown),
        /* Bin mode: general tests */
        ztest_unit_test_setup_teardown(test_bin_num_elem, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_serialize_long_string, setup, teardown),
#ifdef CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT
        /* Bin mode: binary (bytes) data type */
        ztest_unit_test_setup_teardown(test_bin_serialize_bytes, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_deserialize_bytes, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_patch_fetch_bytes, setup, teardown),
#endif
        /* Bin mode: exporting/importing of data */
        ztest_unit_test_setup_teardown(test_bin_export, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_import, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_import_record, setup, teardown),
        /* Bin mode: update notification */
        ztest_unit_test_setup_teardown(test_bin_update_callback, setup, teardown),
        /* Bin mode: request paths by IDs and vice versa */
        ztest_unit_test_setup_teardown(test_bin_fetch_paths, setup, teardown),
        ztest_unit_test_setup_teardown(test_bin_fetch_ids, setup, teardown)
    );

    ztest_run_test_suite(thingset_tests);
}
