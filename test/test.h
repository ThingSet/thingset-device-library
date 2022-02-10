/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet unit testing (test interface).
 */

#ifndef THINGSET_TEST_H
#define THINGSET_TEST_H

/**
 * @brief ThingSet unit testing.
 *
 * @defgroup ts_test_api ThingSet unit testing (test interface).
 * @{
 */

/* Public APIs */
#include "../src/thingset.h"

/* Private APIs */
#include "../src/ts_macro.h"
#include "../src/ts_jsmn.h"
#include "../src/ts_buf.h"
#include "../src/ts_port.h"
#include "../src/ts_msg.h"
#include "../src/ts_obj.h"
#include "../src/ts_ctx.h"
#include "../src/ts_app.h"
#include "../apps/shell/ts_shell.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Test support
 * ------------
 *
 * Implemented in test_support.c
 */

/**
 * @def TEST_ASSERT_BUFFER_SIZE
 *
 * @brief Size of buffer for assertion message build up.
 */
#define TEST_ASSERT_BUFFER_SIZE 300

/**
 * @brief Buffer for assertion message build up.
 *
 * @note Usage depends on choosen test framwork.
 */
extern char test_assert_buffer[TEST_ASSERT_BUFFER_SIZE];

/**
 * @brief Current test file name.
 *
 * @note Usage depends on choosen test framwork.
 */
extern const char *test_file_current;

/**
 * @brief Current test function name.
 *
 * @note Usage depends on choosen test framwork.
 */
extern const char *test_func_current;


/**
 * @def TEST_LOG_BUFFER_SIZE
 *
 * @brief Size of buffer for logging during test.
 */
#define TEST_LOG_BUFFER_SIZE 3000

/**
 * @brief Buffer for logging during test - used mostly for message logging */
extern char test_log_buf[TEST_LOG_BUFFER_SIZE];

#define TEST_REQ_BUFFER_LEN 500
#define TEST_RESP_BUFFER_LEN 500

extern uint8_t test_req_buf[TEST_REQ_BUFFER_LEN];
extern uint8_t test_resp_buf[TEST_RESP_BUFFER_LEN];

extern struct ts_array_info pub_serial_array;

/*
 * Common test data
 * ----------------
 *
 * Implemented in test_data.c
 */

/* Categories / first layer object IDs */
#define ID_ROOT     0x00
#define ID_INFO     0x01        // read-only device information (e.g. manufacturer, device ID)
#define ID_MEAS     0x02        // output data (e.g. measurement values)
#define ID_STATE    0x03        // recorded data (history-dependent)
#define ID_REC      0x04        // recorded data (history-dependent)
#define ID_INPUT    0x05        // input data (e.g. set-points)
#define ID_CONF     0x06        // configurable settings
#define ID_CAL      0x07        // calibration
#define ID_REPORT   0x0A        // reports
#define ID_DFU      0x0D        // device firmware upgrade
#define ID_RPC      0x0E        // remote procedure calls
#define ID_PUB      0x0F        // publication setup

#define SUBSET_REPORT  (1U << 0)   // report subset of data items for publication
#define SUBSET_CAN     (1U << 1)   // data nodes used for CAN bus publication messages

#define TEST_MANUFACTURER "Libre Solar"
#define TEST_DEVICE_ID_INSTANCE "ABCD1234"
#define TEST_DEVICE_ID_NEIGHBOUR "ABCD5678"

extern bool pub_report_enable;
extern uint16_t pub_report_interval;
extern uint16_t pub_serial_interval;
extern bool pub_can_enable;
extern uint16_t pub_can_interval;
extern char auth_password[11];
extern char strbuf[300];
extern float f32;
extern int32_t i32;
extern bool b;
extern int32_t A[100];
extern struct ts_array_info int32_array;
extern float B[100];
extern struct ts_array_info float32_array;
extern uint8_t bytes[300];
extern struct ts_bytes_buffer bytes_buf;

/*
 * All the ports used for testing
 * ------------------------------
 *
 * - 0: "instance" app port
 * - 1: "neighbour" app port
 * - 2: shell application port
 * - 3: loopback a port
 * - 4: loopback b port
 */

#define TEST_PORT_COUNT         5

/*
 * Local contexts used for testing
 * -------------------------------
 */

#define TEST_CORE_LOCID         0   /* TS_CONFIG_CORE_LOCID  is 0 by default */
#define TEST_INSTANCE_LOCID     1
#define TEST_NEIGHBOUR_LOCID    2

#define TEST_INSTANCE_UID       0x1111111111111111ULL;
#define TEST_NEIGHBOUR_UID      0x2222222222222222ULL;

extern struct ts_ctx_core_data TS_CAT(ts_ctx_core_data_, TEST_CORE_LOCID);
extern struct ts_ctx_com_data TS_CAT(ts_ctx_com_data_, TEST_INSTANCE_LOCID);
extern struct ts_ctx_com_data TS_CAT(ts_ctx_com_data_, TEST_NEIGHBOUR_LOCID);
extern const struct ts_ctx_core TS_CAT(ts_ctx_core_, TEST_CORE_LOCID);
extern const struct ts_ctx_com TS_CAT(ts_ctx_com_, TEST_INSTANCE_LOCID);
extern const struct ts_ctx_com TS_CAT(ts_ctx_com_, TEST_NEIGHBOUR_LOCID);

/*
 * Applications used for testing
 * -----------------------------
 *
 * Implemented in test_data.c
 */

/* Dummy applications */

struct test_app_config;
struct test_app_data;

#define TEST_APP_INSTANCE_PORTID    0
#define TEST_APP_INSTANCE_LOCID     TEST_INSTANCE_LOCID
#define TEST_APP_INSTANCE_NAME      "test_app_instance"

extern const struct thingset_port TS_CAT(ts_port_, TEST_APP_INSTANCE_PORTID);
extern const struct thingset_app TS_CAT(ts_port_config_, TEST_APP_INSTANCE_PORTID);
extern struct ts_app_port_data TS_CAT(ts_port_data_, TEST_APP_INSTANCE_PORTID);

#define TEST_APP_NEIGHBOUR_PORTID   1
#define TEST_APP_NEIGHBOUR_LOCID    TEST_NEIGHBOUR_LOCID
#define TEST_APP_NEIGHBOUR_NAME     "test_app_neighbour"

extern const struct thingset_port TS_CAT(ts_port_, TEST_APP_NEIGHBOUR_PORTID);
extern const struct thingset_app TS_CAT(ts_port_config_, TEST_APP_NEIGHBOUR_PORTID);
extern struct ts_app_port_data TS_CAT(ts_port_data_, TEST_APP_NEIGHBOUR_PORTID);

/* Shell application used in tests (Zephyr only at the moment) */
#define TEST_SHELL_PORTID           2
#define TEST_SHELL_LOCID            TEST_INSTANCE_LOCID
#define TEST_SHELL_NAME             "test_shell"

extern const struct thingset_port TS_CAT(ts_port_, TEST_SHELL_PORTID);
extern const struct thingset_app TS_CAT(ts_port_config_, TEST_SHELL_LOCID);
extern struct ts_app_port_data TS_CAT(ts_port_data_, TEST_SHELL_LOCID);

/*
 * Ports used for testing
 * ----------------------
 *
 * Implemented in test_data.c
 */

extern thingset_uid_t test_uid_instance;
extern thingset_uid_t test_uid_neighbour;

#include "../src/ports/loopback_simple/loopback_simple.h"
extern const struct ts_port_api TS_CAT(loopback_simple, _api);

#define TEST_PORT_LOOPBACK_A_PORTID     3
#define TEST_PORT_LOOPBACK_A_LOCID      TEST_INSTANCE_LOCID
#define TEST_PORT_LOOPBACK_A_NAME       "test_port_loopback_a"

extern const struct thingset_port TS_CAT(ts_port_, TEST_PORT_LOOPBACK_A_PORTID);
extern const THINGSET_PORT_CONFIG_STRUCT(loopback_simple)
                                        TS_CAT(ts_port_config_, TEST_PORT_LOOPBACK_A_PORTID);
extern THINGSET_PORT_DATA_STRUCT(loopback_simple)
                                        TS_CAT(ts_port_data_, TEST_PORT_LOOPBACK_A_PORTID);

#define TEST_PORT_LOOPBACK_B_PORTID     4
#define TEST_PORT_LOOPBACK_B_LOCID      TEST_INSTANCE_LOCID
#define TEST_PORT_LOOPBACK_B_NAME       "test_port_loopback_b"

extern const struct thingset_port TS_CAT(ts_port_, TEST_PORT_LOOPBACK_B_PORTID);
extern const THINGSET_PORT_CONFIG_STRUCT(loopback_simple)
                                        TS_CAT(ts_port_config_, TEST_PORT_LOOPBACK_B_PORTID);
extern THINGSET_PORT_DATA_STRUCT(loopback_simple)
                                        TS_CAT(ts_port_data_, TEST_PORT_LOOPBACK_B_PORTID);

/*
 * Helper Functions
 * ----------------
 *
 * Implemented in test_support.c
 */

/* Test core context */
extern bool test_core_conf_callback_called;
extern bool test_core_dummy_called;
extern void test_core_dummy_function(void);
extern void test_core_conf_callback(void);
extern void test_core_reset_function(void);
extern void test_core_auth_function(void);

/* Test communication context */
extern bool test_instance_conf_callback_called;
extern bool test_instance_dummy_called;
extern void test_instance_dummy_function(void);
extern void test_instance_conf_callback(void);
extern void test_instance_reset_function(void);
extern void test_instance_auth_function(void);

extern void test_neighbour_reset_function(void);
/*
 * Helpers and complex assert functions
 * ------------------------------------
 *
 * Implemented in test_support.c
 */

/**
 * @def TEST_ASSERT_EXEC
 *
 * @brief Support macro for complex TEST_ASSERT_xxx macros.
 *
 * @note May be re-defined by the specific test environment.
 */
#ifndef TEST_ASSERT_EXEC
#define TEST_ASSERT_EXEC(...)                                                                   \
    do {                                                                                        \
        const char * test_file_save = test_file_current;                                        \
        const char * test_func_save = test_func_current;                                        \
        test_file_current = __FILE__;                                                           \
        test_func_current = __func__;                                                           \
        __VA_ARGS__;                                                                            \
        test_file_current = test_file_save;                                                     \
        test_func_current = test_func_save;                                                     \
    } while(false)
#endif

extern int _hex2bin(uint8_t *bin, size_t bin_size, const char *hex);

/* complex assert helpers - work on 'core' context */

void assert_bin_resp(const uint8_t *test_req_buf, int resp_len, const char *exp_hex,
                     unsigned int assert_line, const char *assert_msg);

void assert_bin_req(const uint8_t *req_b, int req_len, const char *exp_hex,
                    unsigned int assert_line, const char *assert_msg);

void assert_bin_req_exp_bin(const uint8_t *req_b, int req_len, const uint8_t *exp_b, int exp_len,
                            unsigned int assert_line, const char *assert_msg);

void assert_bin_req_hex(const char *req_hex, const char *exp_hex,
                        unsigned int assert_line, const char *assert_msg);

void assert_txt_resp(int exp_len, const char *exp_s, const char*msg);
void assert_txt_req(const char *req_s, const char *exp_s, const char*msg);
void assert_json2cbor(char const *name, char const *json_value, uint16_t id, const char *const cbor_value_hex, const char*msg);
void assert_cbor2json(char const *name, char const *json_value, uint16_t id, char const *cbor_value_hex, const char*msg);

/* complex assert helpers - work on communication context "instance" */

/** @brief Number of bytes at start of message log to skip when comparing to expected result */
#define ASSERT_MSG_LOG_SKIP 15

void assert_msg_add_response_get_cbor(struct thingset_msg *response, struct thingset_msg *request,
                                      const char *object_path, int expected_ret,
                                      const char *expected_str,
                                      unsigned int assert_line, const char *assert_msg);

void assert_msg_add_response_get_json(struct thingset_msg *response, struct thingset_msg *request,
                                      ts_obj_id_t obj_id, int expected_ret,
                                      const char *expected_str,
                                      unsigned int assert_line, const char *assert_msg);

void assert_msg_fetch_cbor(struct thingset_msg *request, struct thingset_msg *response,
                           const char *object_path,
                           bool test_object_id,
                           bool test_object_ids, uint16_t object_count, const char **object_names,
                           int expected_ret_add_request, const char *expected_str_add_request,
                           int expected_ret_pull_request, uint8_t expected_status_id_pull_request,
                           int expected_ret_add_response, const char *expected_str_add_response,
                           unsigned int assert_line, const char *assert_msg);

/*
 * Test asserts
 * ------------
 */

#define ASSERT_MSG(assert_func) \
        _ASSERT_MSG("Assertion triggered by " assert_func )
#define _ASSERT_MSG(assert_msg) assert_msg

/**
 * @def TEST_ASSERT_BIN_RESP
 *
 * @brief Assert binary response message content.
 *
 * @param[in] resp Response message buffer.
 * @param[in] resp_len Response message length.
 * @param[in] exp Expected response message content defined as string with hex bytes definitions.
 */
#define TEST_ASSERT_BIN_RESP(resp, resp_len, exp)                                               \
    TEST_ASSERT_EXEC(                                                                           \
        assert_bin_resp(resp, resp_len, exp, __LINE__,                                          \
                        ASSERT_MSG("TEST_ASSERT_BIN_RESP(" #resp ", " #resp_len ", " #exp ")")) \
    )

/**
 * @def TEST_ASSERT_BIN_REQ
 *
 * @brief Assert reponse on binary request message.
 *
 * @note Works on 'core' ThingSet context.
 *
 * @param[in] req Request message buffer.
 * @param[in] req_len Request message length.
 * @param[in] exp Expected response message content defined as string with hex bytes definitions.
 */
#define TEST_ASSERT_BIN_REQ(req, req_len, exp)                                                  \
    TEST_ASSERT_EXEC(                                                                           \
        assert_bin_req(req, req_len, exp, __LINE__,                                             \
                       ASSERT_MSG("TEST_ASSERT_BIN_REQ(" #req ", " #req_len ", " #exp ")"))     \
    )

/**
 * @def TEST_ASSERT_BIN_REQ_EXP_BIN
 *
 * @brief Assert response on binary request message.
 *
 * @note Works on 'core' ThingSet context.
 *
 * @param[in] req Request message buffer.
 * @param[in] req_len Request message length.
 * @param[in] exp Expected binary reponse message.
 * @param[in] exp_len Expected binary response message length.
 */
#define TEST_ASSERT_BIN_REQ_EXP_BIN(req, req_len, exp, exp_len)                                 \
    TEST_ASSERT_EXEC(                                                                           \
        assert_bin_req_exp_bin(req, req_len, exp, exp_len, __LINE__,                            \
        ASSERT_MSG("TEST_ASSERT_BIN_REQ_EXP_BIN(" #req ", " #req_len ", " #exp ", " #exp_len ")")) \
    )

/**
 * @def TEST_ASSERT_BIN_REQ_HEX
 *
 * @brief Assert reponse on binary request message.
 *
 * @note Works on 'core' ThingSet context.
 *
 * @param[in] req Request message content defined as string with hex bytes definitions.
 * @param[in] exp Expected response message content defined as string with hex bytes definitions.
 */
#define TEST_ASSERT_BIN_REQ_HEX(req, exp)                                                       \
    TEST_ASSERT_EXEC(                                                                           \
        assert_bin_req_hex(req, exp, __LINE__,                                                  \
                           ASSERT_MSG("TEST_ASSERT_BIN_REQ_HEX(" #req ", " #exp ")"))           \
    )

#define TEST_ASSERT_TXT_RESP(len, resp) \
        assert_txt_resp(len, resp, ASSERT_MSG("TEST_ASSERT_TXT_RESP(" #len ", " #resp ")"))
#define TEST_ASSERT_TXT_REQ(req, exp) \
        assert_txt_req(req, exp, ASSERT_MSG("TEST_ASSERT_TXT_REQ(" #req ", " #exp ")"))
#define TEST_ASSERT_JSON2CBOR(name, json_value, id, cbor_hex_value) \
        assert_json2cbor(name, json_value, id, cbor_hex_value, ASSERT_MSG("TEST_ASSERT_JSON2CBOR(" #name ", " #json_value ", " #id ", " #cbor_hex_value ")"))
#define TEST_ASSERT_CBOR2JSON(name, json_value, id, cbor_hex_value) \
        assert_cbor2json(name, json_value, id, cbor_hex_value, ASSERT_MSG("TEST_ASSERT_CBOR2JSON(" #name ", " #json_value ", " #id ", " #cbor_hex_value ")"))

/**
 * @def TEST_ASSERT_MSG_ADD_RESPONSE_GET_CBOR
 *
 * @brief Assert ts_msg_add_response_get_cbor()
 *
 * @note Works on 'instance' ThingSet communication context.
 *
 * @param[in] tc test case string
 * @param[in] response communication message buffer to add response to
 * @param[in] request message buffer that contains a (partial) request.
 *                    Any of TS_MSG_CODE_REQUEST_GET_VALUES, TS_MSG_CODE_REQUEST_GET_IDS,
 *                    TS_MSG_CODE_REQUEST_GET_NAMES_VALUES shall be set in message status.
 * @param[in] obj_id Object id
 * @param[in] expected_ret Expected return value
 * @param[in] expected_str Expected message content as printed by ts_msg_log().
 */
#define TEST_ASSERT_MSG_ADD_RESPONSE_GET_CBOR(tc, response, request, obj_id,                    \
                                              expected_ret, expected_str)                       \
    TEST_ASSERT_EXEC(                                                                           \
        assert_msg_add_response_get_cbor(response, request, obj_id, expected_ret, expected_str, \
            __LINE__, ASSERT_MSG("TEST_ASSERT_MSG_ADD_RESPONSE_GET_CBOR(" #tc ", ...)"))        \
    )

/**
 * @def TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON
 *
 * @brief Assert ts_msg_add_response_get_json()
 *
 * @note Works on 'instance' ThingSet communication context.
 *
 * @param[in] tc test case string
 * @param[in] response communication message buffer to add response to
 * @param[in] request message buffer that contains a (partial) request.
 *                    Any of TS_MSG_CODE_REQUEST_GET_VALUES, TS_MSG_CODE_REQUEST_GET_IDS,
 *                    TS_MSG_CODE_REQUEST_GET_NAMES_VALUES shall be set in message status.
 * @param[in] object_path Path to object
 * @param[in] expected_ret Expected return value
 * @param[in] expected_str Expected message content as printed by ts_msg_log().
 */
#define TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON(tc, response, request, object_path,               \
                                              expected_ret, expected_str)                       \
    TEST_ASSERT_EXEC(                                                                           \
        assert_msg_add_response_get_json(response, request, object_path, expected_ret,          \
            expected_str, __LINE__,                                                             \
            ASSERT_MSG("TEST_ASSERT_MSG_ADD_RESPONSE_GET_JSON(" #tc ", ...)"))                  \
    )

/**
 * @def TEST_ASSERT_MSG_FETCH_CBOR
 *
 * @brief Assert ts_msg_add_request_fetch_cbor() -> ts_msg_pull_request_cbor() -> ts_msg_add_response_fetch_cbor()
 *
 * @note Works on 'instance' ThingSet communication context.
 *
 * @param[in] tc test case string
 * @param[in] request communication message buffer
 * @param[in] response communication message buffer
 * @param[in] object_path Object path
 * @param[in] test_object_id Test with single object id defined by object path
 * @param[in] test_object_ids Test with several object ids defined by object names
 * @param[in] object_count Number of object ids to test with
 * @param[in] object_names Pointer to array of object names.
 * @param[in] expected_ret_add_request Expected return value of ts_msg_add_request_fetch_cbor().
 * @param[in] expected_str_add_request Expected message content of request message of
 *                                     ts_msg_add_request_fetch_cbor() as printed by ts_msg_log().
 * @param[in] expected_ret_pull_request Expected return value of ts_msg_pull_request_cbor().
 * @param[in] expected_status_id_pull_request Expected message status id returned
 *                                            by ts_msg_pull_request_cbor().
 * @param[in] expected_ret_add_response Expected return value of ts_msg_add_response_fetch_cbor().
 * @param[in] expected_str_add_response Expected message content reponse message of
 *                                      ts_msg_add_response_fetch_cbor() as printed by ts_msg_log().
 */
#define TEST_ASSERT_MSG_FETCH_CBOR(tc, request, response, object_path, test_object_id,          \
                                   test_object_ids, object_count, object_names,                 \
                                   expected_ret_add_request, expected_str_add_request,          \
                                   expected_ret_pull_request, expected_status_id_pull_request,  \
                                   expected_ret_add_response, expected_str_add_response)        \
    TEST_ASSERT_EXEC(                                                                           \
        assert_msg_fetch_cbor(request, response, object_path, test_object_id,                   \
                              test_object_ids, object_count, object_names,                      \
                               expected_ret_add_request, expected_str_add_request,              \
                               expected_ret_pull_request, expected_status_id_pull_request,      \
                               expected_ret_add_response, expected_str_add_response,            \
                               __LINE__,                                                        \
                               ASSERT_MSG("TEST_ASSERT_MSG_FETCH_CBOR(" #tc ", ...)"))          \
    )

/*
 * Test initialisation
 * ---------------------
 *
 * Implemented in test_data.c
 */

/**
 * @brief Initialise test data to predefined values.
 */
void test_init_test_data(void);

/**
 * @brief Setup for test case as used by Unity Test.
 */
void setUp(void);

/**
 * @brief Teardown for test case as used by Unity Test.
 */
void tearDown(void);

/*
 * Test functions
 * --------------
 */

void test_assert(void);
void test_txt_patch_bin_fetch(void);
void test_bin_patch_txt_fetch(void);

void test_txt_get_meas_names(void);
void test_txt_get_meas_names_values(void);
void test_txt_get_single_value(void);
void test_txt_fetch_array(void);
void test_txt_fetch_rounded(void);
void test_txt_fetch_nan(void);
void test_txt_fetch_inf(void);
void test_txt_fetch_int32_array(void);
void test_txt_fetch_float_array(void);
void test_txt_patch_wrong_data_structure(void);
void test_txt_patch_array(void);
void test_txt_patch_readonly(void);
void test_txt_patch_wrong_path(void);
void test_txt_patch_unknown_object(void);
void test_txt_conf_callback(void);
void test_txt_exec(void);
void test_txt_statement_subset(void);
void test_txt_statement_group(void);
void test_txt_pub_list_channels(void);
void test_txt_pub_enable(void);
void test_txt_pub_delete_append_object(void);
void test_txt_auth_user(void);
void test_txt_auth_root(void);
void test_txt_auth_long_password(void);
void test_txt_auth_failure(void);
void test_txt_auth_reset(void);
void test_txt_wrong_command(void);
void test_txt_get_endpoint(void);
void test_txt_export(void);

void test_bin_get_meas_ids_values(void);
void test_bin_get_meas_names_values(void);
void test_bin_get_single_value(void);
void test_bin_patch_multiple_objects(void);
void test_bin_fetch_meas_ids(void);
void test_bin_fetch_meas_names(void);
void test_bin_fetch_multiple_objects(void);
void test_bin_patch_float_array(void);
void test_bin_fetch_float_array(void);
void test_bin_patch_rounded_float(void);
void test_bin_fetch_rounded_float(void);
void test_bin_statement_subset(void);
void test_bin_statement_group(void);
void test_bin_exec(void);
void test_bin_patch_fetch_bytes(void);
void test_bin_export(void);
void test_bin_import(void);

void test_buf(void);
void test_time(void);
void test_ctx_com(void);
void test_ctx_com_port(void);
void test_ctx_com_node(void);
void test_ctx_core(void);
void test_msg_add_primitives(void);
void test_msg_add_response_cbor(void);
void test_msg_add_response_json(void);
void test_msg_add_statement_cbor(void);
void test_msg_add_statement_json(void);
void test_msg_alloc(void);
void test_msg_pull_request_get_cbor(void);
void test_msg_pull_request_get_json(void);
void test_msg_pull_request_fetch_json(void);
void test_msg_pull_request_patch_json(void);
void test_msg_pull_request_exec_json(void);
void test_msg_pull_request_create_json(void);
void test_msg_pull_request_delete_json(void);
void test_msg_pull_request_invalid_json(void);
void test_obj_db_init(void);
void test_jsmn(void);

/*
 * Test suites/ sets
 * -----------------
 */

void tests_assert(void);
void tests_time(void);
void tests_buf(void);
void tests_jsmn(void);
void tests_obj(void);
void tests_ctx(void);
void tests_msg(void);
void tests_txt(void);
void tests_bin(void);
void tests_common(void);
void tests_shell(void);

/**
 * @brief Run ThingSet implementation specific test suite.
 *
 * @note tests_impl() to be provided by the implementation.
 */
void tests_impl(void);

#ifdef __cplusplus
} /* extern "C" */

/*
 * C++ test functions
 * ------------------
 */
void test_shim_get_object(void);

/*
 * C++ Test suites/ sets
 * ---------------------
 */
void tests_shim(void);

#endif

/**
 * @} <!-- ts_test_api -->
 */

#endif /* THINGSET_TEST_H */
