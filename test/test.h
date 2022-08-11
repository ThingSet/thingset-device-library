/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_TEST_H
#define THINGSET_TEST_H

#include "../src/thingset_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

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
#define ID_NESTED   0x09        // dummy data for tests
#define ID_REPORT   0x0A        // reports
#define ID_DFU      0x0D        // device firmware upgrade
#define ID_RPC      0x0E        // remote procedure calls
#define ID_PUB      0x0F        // publication setup

#define SUBSET_REPORT  (1U << 0)   // report subset of data items for publication
#define SUBSET_CAN     (1U << 1)   // data nodes used for CAN bus publication messages
#define SUBSET_NVM     (1U << 2)   // data that should be stored in EEPROM
#define SUBSET_NESTED  (1U << 3)

extern char manufacturer[];
extern bool pub_report_enable;
extern uint16_t pub_serial_interval;
extern bool pub_can_enable;
extern uint16_t pub_can_interval;
extern char auth_password[11];
extern char strbuf[300];
extern float f32;
extern int32_t i32;
extern bool b;
extern int32_t A[100];
extern struct ts_array int32_array;
extern float B[100];
extern struct ts_array float32_array;
extern uint8_t bytes[300];
extern struct ts_bytes_buffer bytes_buf;
extern struct ts_data_object data_objects[];
extern size_t data_objects_size;

/*
 * Context used for testing
 * ------------------------
 *
 * Implemented in test_context.c
 */
#define TS_REQ_BUFFER_LEN 500
#define TS_RESP_BUFFER_LEN 500

extern struct ts_context ts;
extern uint8_t req_buf[TS_REQ_BUFFER_LEN];
extern uint8_t resp_buf[TS_RESP_BUFFER_LEN];

extern bool group_callback_called;
extern bool update_callback_called;
extern bool dummy_called_flag;
extern struct ts_array pub_serial_array;

/* Helper functions (see test_context.c) */
void dummy(void);
void group_callback(void);
void update_callback(void);
void reset_function(void);
void auth_function(void);
int _hex2bin(uint8_t *bin, size_t bin_size, const char *hex);

/* complex assert helpers (see test.context.c) */
void assert_bin_resp(const uint8_t *resp_buf, int resp_len, const char *exp_hex, const char*msg);
void assert_bin_req(const uint8_t *req_b, int req_len, const char *exp_hex, const char*msg);
void assert_bin_req_exp_bin(const uint8_t *req_b, int req_len, const uint8_t *exp_b, int exp_len, const char*msg);
void assert_bin_req_hex(const char *req_hex, const char *exp_hex, const char*msg);
void assert_txt_resp(int exp_len, const char *exp_s, const char*msg);
void assert_txt_req(const char *req_s, const char *exp_s, const char*msg);
void assert_json2cbor(char const *name, char const *json_value, uint16_t id, const char *const cbor_value_hex, const char*msg);
void assert_cbor2json(char const *name, char const *json_value, uint16_t id, char const *cbor_value_hex, const char*msg);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

#ifndef STRINGIFY
#define STRINGIFY(x) _STRINGIFY(x)
#define _STRINGIFY(x) #x
#endif

#define ASSERT_MSG(assert_func) \
        _ASSERT_MSG("Assertion triggered by " assert_func " at #" STRINGIFY(__LINE__) " " __FILE__)
#define _ASSERT_MSG(assert_msg) assert_msg

#define TEST_ASSERT_BIN_RESP(resp, resp_len, exp) \
        assert_bin_resp(resp, resp_len, exp, ASSERT_MSG("TEST_ASSERT_BIN_RESP(" #resp ", " #resp_len ", " #exp ")"))
#define TEST_ASSERT_BIN_REQ(req, req_len, exp) \
        assert_bin_req(req, req_len, exp, ASSERT_MSG("TEST_ASSERT_BIN_REQ(" #req ", " #req_len ", " #exp ")"))
#define TEST_ASSERT_BIN_REQ_EXP_BIN(req, req_len, exp, exp_len) \
        assert_bin_req_exp_bin(req, req_len, exp, exp_len, ASSERT_MSG("TEST_ASSERT_BIN_REQ_EXP_BIN(" #req ", " #req_len ", " #exp ", " #exp_len ")"))
#define TEST_ASSERT_BIN_REQ_HEX(req, exp) \
        assert_bin_req_hex(req, exp, ASSERT_MSG("TEST_ASSERT_BIN_REQ_HEX(" #req ", " #exp ")"))
#define TEST_ASSERT_TXT_RESP(len, resp) \
        assert_txt_resp(len, resp, ASSERT_MSG("TEST_ASSERT_TXT_RESP(" #len ", " #resp ")"))
#define TEST_ASSERT_TXT_REQ(req, exp) \
        assert_txt_req(req, exp, ASSERT_MSG("TEST_ASSERT_TXT_REQ(" #req ", " #exp ")"))
#define TEST_ASSERT_JSON2CBOR(name, json_value, id, cbor_hex_value) \
        assert_json2cbor(name, json_value, id, cbor_hex_value, ASSERT_MSG("TEST_ASSERT_JSON2CBOR(" #name ", " #json_value ", " #id ", " #cbor_hex_value ")"))
#define TEST_ASSERT_CBOR2JSON(name, json_value, id, cbor_hex_value) \
        assert_cbor2json(name, json_value, id, cbor_hex_value, ASSERT_MSG("TEST_ASSERT_CBOR2JSON(" #name ", " #json_value ", " #id ", " #cbor_hex_value ")"))

/*
 * Test functions
 * --------------
 *
 * Implemented in test_txt.c, test_bin.c, test_common.c
 */

void test_assert(void);
void test_txt_patch_bin_fetch(void);
void test_bin_patch_txt_fetch(void);
void test_ts_init(void);

void test_txt_get_meas_names(void);
void test_txt_get_meas_names_values(void);
void test_txt_get_single_value(void);
void test_txt_fetch_array(void);
void test_txt_fetch_rounded(void);
void test_txt_fetch_nan(void);
void test_txt_fetch_inf(void);
void test_txt_fetch_int32_array(void);
void test_txt_fetch_float_array(void);
void test_txt_fetch_num_records(void);
void test_txt_fetch_record(void);
void test_txt_fetch_records_object(void);
void test_txt_patch_wrong_data_structure(void);
void test_txt_patch_array(void);
void test_txt_patch_readonly(void);
void test_txt_patch_wrong_path(void);
void test_txt_patch_unknown_object(void);
void test_txt_group_callback(void);
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
void test_txt_update_callback(void);

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
void test_bin_fetch_num_records(void);
void test_bin_fetch_record(void);
void test_bin_fetch_record_item(void);
void test_bin_fetch_by_name(void);
void test_bin_statement_subset(void);
void test_bin_statement_group(void);
void test_bin_pub_can(void);
void test_bin_exec(void);
void test_bin_num_elem(void);
void test_bin_serialize_long_string(void);
void test_bin_serialize_bytes(void);
void test_bin_deserialize_bytes(void);
void test_bin_patch_fetch_bytes(void);
void test_bin_export(void);
void test_bin_import(void);
void test_bin_update_callback(void);
void test_bin_fetch_paths(void);
void test_bin_fetch_ids(void);

#ifdef __cplusplus
} /* extern "C" */

/*
 * Implemented in test_shim.cpp
 */
void test_shim_get_object(void);

#endif

#endif /* THINGSET_TEST_H */
