

#include "thingset.h"
//#include "cbor.h"
#include "unity.h"
//#include <sys/types.h>  // for definition of endianness

#include "test_data.h"

measurement_data_t meas;
calibration_data_t cal;

#include "tests_json.h"
#include "tests_cbor.h"
#include "tests_common.h"

int main()
{
    test_data_init();

    UNITY_BEGIN();

    RUN_TEST(json_wrong_command);

    RUN_TEST(json_write_wrong_data_structure);
    RUN_TEST(json_write_array);
    RUN_TEST(json_write_float);     // write 54.3 to maximum voltage value
    RUN_TEST(json_write_int);       // write 61 to dcdc restart interval
    RUN_TEST(json_write_readonly);  // attempt to write read-only value
    RUN_TEST(json_write_unknown);   // attempt to write non-existing value

    RUN_TEST(json_read_float);     // read maximum voltage value (test if changed to 54.3)
    RUN_TEST(json_read_array);

    RUN_TEST(json_list_input);
    RUN_TEST(json_list_all);

    // data conversion tests
    RUN_TEST(write_json_read_cbor);
    RUN_TEST(write_cbor_read_json);

    UNITY_END();
}

