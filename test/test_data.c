/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

/* Provide for inclusion in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

// info
char manufacturer[] = "Libre Solar";
static uint32_t timestamp = 12345678;
static char device_id[] = "ABCD1234";

// conf
static float bat_charging_voltage = 14.4;
static float load_disconnect_voltage = 10.8;

// input
static bool enable_switch = false;

// meas
static float battery_voltage = 14.1;
static float battery_current = 5.13;
static int16_t ambient_temp = 22;

// rec
static float bat_energy_hour = 32.2;
static float bat_energy_day = 123;
static int16_t ambient_temp_max_day = 28;

// pub
bool pub_report_enable = false;
uint16_t pub_report_interval = 1000;
bool pub_info_enable = true;

// exec
void reset_function(void);
void auth_function(void);
char auth_password[11];

char strbuf[300];

float f32;
int32_t decfrac;

static uint64_t ui64;
static int64_t i64;

static uint32_t ui32;
int32_t i32;

static uint16_t ui16;
static int16_t i16;

bool b;

int32_t A[100] = {4, 2, 8, 4};
struct ts_array_info int32_array = {A, sizeof(A)/sizeof(int32_t), 4, TS_T_INT32};

float B[100] = {2.27, 3.44};
struct ts_array_info float32_array = {B, sizeof(B)/sizeof(float), 2, TS_T_FLOAT32};

uint8_t bytes[300] = {};
struct ts_bytes_buffer bytes_buf = { bytes, 0 };

void dummy(void);
void conf_callback(void);


struct ts_data_object data_objects[] = {

    // DEVICE INFORMATION /////////////////////////////////////////////////////

    TS_GROUP(ID_INFO, "info", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_STRING(0x19, "Manufacturer", manufacturer, 0,
        ID_INFO, TS_ANY_R, 0),

    TS_ITEM_UINT32(0x1A, "Timestamp_s", &timestamp,
        ID_INFO, TS_ANY_RW, SUBSET_REPORT),

    TS_ITEM_STRING(0x1B, "DeviceID", device_id, sizeof(device_id),
        ID_INFO, TS_ANY_R | TS_MKR_W, 0),

    // CONFIGURATION //////////////////////////////////////////////////////////

    TS_GROUP(ID_CONF, "conf", &conf_callback, ID_ROOT),

    TS_ITEM_FLOAT(0x31, "BatCharging_V", &bat_charging_voltage, 2,
        ID_CONF, TS_ANY_RW, 0),

    TS_ITEM_FLOAT(0x32, "LoadDisconnect_V", &load_disconnect_voltage, 2,
        ID_CONF, TS_ANY_RW, 0),

    // INPUT DATA /////////////////////////////////////////////////////////////

    TS_GROUP(ID_INPUT, "input", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_BOOL(0x61, "EnableCharging", &enable_switch,
        ID_INPUT, TS_ANY_RW, 0),

    // MEASUREMENT DATA ///////////////////////////////////////////////////////

    TS_GROUP(ID_MEAS, "meas", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_FLOAT(0x71, "Bat_V", &battery_voltage, 2,
        ID_MEAS, TS_ANY_R, SUBSET_REPORT | SUBSET_CAN),

    TS_ITEM_FLOAT(0x72, "Bat_A", &battery_current, 2,
        ID_MEAS, TS_ANY_R, SUBSET_REPORT | SUBSET_CAN),

    TS_ITEM_INT16(0x73, "Ambient_degC", &ambient_temp,
        ID_MEAS, TS_ANY_R, SUBSET_REPORT),

    // RECORDED DATA //////////////////////////////////////////////////////////

    TS_GROUP(ID_REC, "rec", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_FLOAT(0xA1, "BatHour_kWh", &bat_energy_hour, 2,
        ID_REC, TS_ANY_R, 0),

    TS_ITEM_FLOAT(0xA2, "BatDay_kWh", &bat_energy_day, 2,
        ID_REC, TS_ANY_R, 0),

    TS_ITEM_INT16(0xA3, "AmbientMaxDay_degC", &ambient_temp_max_day,
        ID_REC, TS_ANY_R, 0),

    // REMOTE PROCEDURE CALLS /////////////////////////////////////////////////

    TS_GROUP(ID_RPC, "rpc", TS_NO_CALLBACK, ID_ROOT),

    TS_FUNCTION(0xE1, "x-reset", &reset_function,
        ID_RPC, TS_ANY_RW),

    TS_FUNCTION(0xE2, "x-auth", &auth_function,
        ID_RPC, TS_ANY_RW),

    TS_ITEM_STRING(0xE3, "Password", auth_password, sizeof(auth_password),
        0xE2, TS_ANY_RW, 0),

    // REPORTS ////////////////////////////////////////////////////////////////

    TS_SUBSET(ID_REPORT, "report", SUBSET_REPORT, ID_ROOT, TS_ANY_RW),

    // PUBLICATION DATA ///////////////////////////////////////////////////////

    TS_GROUP(ID_PUB, ".pub", TS_NO_CALLBACK, ID_ROOT),

    TS_GROUP(0xF1, "report", TS_NO_CALLBACK, ID_PUB),

    TS_ITEM_BOOL(0xF2, "Enable", &pub_report_enable,
        0xF1, TS_ANY_RW, 0),

    TS_ITEM_UINT16(0xF3, "Interval_ms", &pub_report_interval,
        0xF1, TS_ANY_RW, 0),

    TS_GROUP(0xF5, "info", TS_NO_CALLBACK, ID_PUB),

    TS_ITEM_BOOL(0xF6, "OnChange", &pub_info_enable,
        0xF5, TS_ANY_RW, 0),

    // UNIT TEST DATA /////////////////////////////////////////////////////////
    // using IDs >= 0x1000

    TS_GROUP(0x1000, "test", TS_NO_CALLBACK, ID_ROOT),

    TS_ITEM_INT32(0x4001, "i32_readonly", &i32, 0x1000, TS_ANY_R, 0),

    TS_FUNCTION(0x5001, "x-dummy", &dummy, ID_RPC, TS_ANY_RW),

    TS_ITEM_UINT64(0x6001, "ui64", &ui64, ID_CONF, TS_ANY_RW, 0),
    TS_ITEM_INT64(0x6002, "i64", &i64, ID_CONF, TS_ANY_RW, 0),
    TS_ITEM_UINT32(0x6003, "ui32", &ui32, ID_CONF, TS_ANY_RW, 0),
    TS_ITEM_INT32(0x6004, "i32", &i32, ID_CONF, TS_ANY_RW, 0),
    TS_ITEM_UINT16(0x6005, "ui16", &ui16, ID_CONF, TS_ANY_RW, 0),
    TS_ITEM_INT16(0x6006, "i16", &i16, ID_CONF, TS_ANY_RW, 0),
    TS_ITEM_FLOAT(0x6007, "f32", &f32, 2, ID_CONF, TS_ANY_RW, 0),
    TS_ITEM_BOOL(0x6008, "bool", &b, ID_CONF, TS_ANY_RW, 0),

    TS_ITEM_STRING(0x6009, "strbuf", strbuf, sizeof(strbuf), ID_CONF, TS_ANY_RW, 0),

    TS_ITEM_FLOAT(0x600A, "f32_rounded", &f32, 0, ID_CONF, TS_ANY_RW, 0),
    TS_ITEM_DECFRAC(0x600B, "DecFrac_degC", &decfrac, -2, ID_CONF, TS_ANY_RW, 0),

    TS_ITEM_UINT32(0x7001, "secret_expert", &ui32, ID_CONF, TS_ANY_R | TS_EXP_W | TS_MKR_W, 0),
    TS_ITEM_UINT32(0x7002, "secret_maker", &ui32, ID_CONF, TS_ANY_R | TS_MKR_W, 0),
    TS_ITEM_ARRAY(0x7003, "arrayi32", &int32_array, 0, ID_CONF, TS_ANY_RW, 0),
    // data_obj->detail will specify the number of decimal places for float
    TS_ITEM_ARRAY(0x7004, "arrayfloat", &float32_array, 2, ID_CONF, TS_ANY_RW, 0),

    TS_ITEM_BYTES(0x8000, "bytesbuf", &bytes_buf, sizeof(bytes), ID_CONF, TS_ANY_RW, 0),
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

size_t data_objects_size = ARRAY_SIZE(data_objects);

#ifdef __cplusplus
} /** extern "C" */
#endif
