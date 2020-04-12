/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#include "thingset.h"
#include "unity.h"

#include "test_data.h"

#ifdef __WIN32__

// To avoid compilation error of unit tests
void setUp (void) {}
void tearDown (void) {}

#endif

measurement_data_t meas;
calibration_data_t cal;

uint8_t req_buf[TS_REQ_BUFFER_LEN];
uint8_t resp_buf[TS_RESP_BUFFER_LEN];

#include "tests_json.h"
#include "tests_cbor.h"
#include "tests_common.h"

ThingSet ts(data_nodes, sizeof(data_nodes)/sizeof(DataNode));

int main()
{
    //test_data_init();
    ts.set_pub_channels(pub_channels, sizeof(pub_channels)/sizeof(ts_pub_channel_t));

    UNITY_BEGIN();

    // data conversion tests
    RUN_TEST(patch_json_fetch_cbor);
    RUN_TEST(patch_cbor_fetch_json);

    // JSON only tests
    RUN_TEST(json_wrong_command);
    RUN_TEST(json_patch_wrong_data_structure);
    RUN_TEST(json_patch_array);
    RUN_TEST(json_patch_readonly);
    RUN_TEST(json_patch_wrong_path);
    RUN_TEST(json_patch_unknown_node);
    RUN_TEST(json_fetch_array);
    RUN_TEST(json_fetch_rounded);
    RUN_TEST(json_list_input);
    RUN_TEST(json_list_names_values_input);
    RUN_TEST(json_exec);
    RUN_TEST(json_pub_msg);
    RUN_TEST(json_conf_callback);
    RUN_TEST(json_auth_user);
    RUN_TEST(json_auth_root);
    RUN_TEST(json_auth_long_password);
    RUN_TEST(json_auth_failure);
    RUN_TEST(json_auth_reset);
    RUN_TEST(json_pub_list);
    RUN_TEST(json_pub_enable);
    RUN_TEST(json_get_endpoint_node);

    // CBOR only tests
    RUN_TEST(cbor_patch_array);
    RUN_TEST(cbor_fetch_array);
    RUN_TEST(cbor_patch_int32_array);
    RUN_TEST(cbor_fetch_int32_array);

    RUN_TEST(json_fetch_int32_array);

    RUN_TEST(cbor_patch_float_array);
    RUN_TEST(cbor_fetch_float_array);

    RUN_TEST(json_fetch_float_array);

    RUN_TEST(cbor_fetch_rounded);
    RUN_TEST(cbor_patch_rounded);       // writes an integer to float
    RUN_TEST(cbor_list_ids_input);
    RUN_TEST(cbor_list_names_input);
    RUN_TEST(cbor_list_names_values_input);
    RUN_TEST(cbor_pub_msg);
    RUN_TEST(cbor_exec);
    RUN_TEST(cbor_num_elem);
    RUN_TEST(cbor_serialize_long_string);
    //RUN_TEST(cbor_auth);
    //RUN_TEST(cbor_auth_failure);
    //RUN_TEST(cbor_auth_reset);

    UNITY_END();
}

bool dummy_called_flag = 0;

void dummy(void)
{
    dummy_called_flag = 1;
}
