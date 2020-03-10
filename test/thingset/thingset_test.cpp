

#include "thingset.h"
//#include "cbor.h"
#include "unity.h"
//#include <sys/types.h>  // for definition of endianness

#include "test_data.h"

// To avoid compilation error of unit tests
void setUp (void) {}
void tearDown (void) {}

measurement_data_t meas;
calibration_data_t cal;

uint8_t req_buf[TS_REQ_BUFFER_LEN];
uint8_t resp_buf[TS_RESP_BUFFER_LEN];
//ts_buffer_t req, resp;

#include "tests_json.h"
#include "tests_cbor.h"
#include "tests_common.h"

ThingSet ts(dataObjects, sizeof(dataObjects)/sizeof(data_object_t));

int main()
{
    //test_data_init();
    ts.set_pub_channels(pub_channels, sizeof(pub_channels)/sizeof(ts_pub_channel_t));

    UNITY_BEGIN();

    // data conversion tests
    RUN_TEST(write_json_read_cbor);
    RUN_TEST(write_cbor_read_json);

    // JSON only tests
    RUN_TEST(json_wrong_command);
    RUN_TEST(json_write_wrong_data_structure);
    RUN_TEST(json_write_array);
    RUN_TEST(json_write_readonly);
    RUN_TEST(json_write_unknown);
    RUN_TEST(json_wrong_category);
    RUN_TEST(json_read_array);
    RUN_TEST(json_read_rounded);
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

    // CBOR only tests
    RUN_TEST(cbor_write_array);
    RUN_TEST(cbor_read_array);
    RUN_TEST(cbor_write_int32_array); 
    RUN_TEST(cbor_read_int32_array);

    RUN_TEST(json_read_int32_array); // json read for array types

    RUN_TEST(cbor_write_float_array);
    RUN_TEST(cbor_read_float_array);

    RUN_TEST(json_read_float_array); // json read for array types

    RUN_TEST(cbor_read_rounded);
    RUN_TEST(cbor_write_rounded);       // writes an integer to float
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
