

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

char buf_req[TS_REQ_BUFFER_LEN];
char buf_resp[TS_RESP_BUFFER_LEN];
ts_buffer_t req, resp;

int main()
{
    test_data_init();

    // initialize buffers
    req.data.str = buf_req;
    req.size = sizeof(buf_req);
    resp.data.str = buf_resp;
    resp.size = sizeof(buf_resp);

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
    RUN_TEST(json_read_array);
    RUN_TEST(json_list_input);
    RUN_TEST(json_list_all);

    // CBOR only tests
    RUN_TEST(cbor_write_array);
    RUN_TEST(cbor_read_array);
    RUN_TEST(cbor_pub_msg);

    UNITY_END();
}

