

#ifndef DATA_OBJECTS_H
#define DATA_OBJECTS_H

#include <stdint.h>
#include <stdbool.h>
#include "thingset.h"

#define TS_REQ_BUFFER_LEN 500
#define TS_RESP_BUFFER_LEN 500

typedef struct measurement_data_t {
    float battery_voltage;
    float battery_voltage_max;  // to be stored in EEPROM
    float solar_voltage;
    float solar_voltage_max;    // to be stored in EEPROM
    float ref_voltage;
    float dcdc_current;
    float dcdc_current_max;     // to be stored in EEPROM
    float dcdc_current_offset;
    //float input_current;
    float load_current;
    float load_current_max;     // to be stored in EEPROM
    float load_current_offset;
    float bat_current;
    float temp_int;             // °C (internal MCU temperature sensor)
    float temp_int_max;         // °C
    float temp_mosfets;         // °C
    float temp_mosfets_max;     // to be stored in EEPROM
    float temp_battery;         // °C
    bool load_enabled;
    float input_Wh_day;
    float output_Wh_day;
    float input_Wh_total;
    float output_Wh_total;
    int num_full_charges;
    int num_deep_discharges;
    int soc;
} measurement_data_t;
extern measurement_data_t meas;

typedef struct calibration_data_t {
    float dcdc_current_min;  // A     --> if lower, charger is switched off
    float dcdc_current_max;
    float load_current_max;
    bool load_overcurrent_flag;
    float solar_voltage_max; // V
    int32_t dcdc_restart_interval; // s    --> when should we retry to start charging after low solar power cut-off?
    float solar_voltage_offset_start; // V  charging switched on if Vsolar > Vbat + offset
    float solar_voltage_offset_stop;  // V  charging switched off if Vsolar < Vbat + offset
    int32_t thermistor_beta_value;  // typical value for Semitec 103AT-5 thermistor: 3435
    bool load_enabled_target;
    bool usb_enabled_target;
    bool pub_data_enabled;
} calibration_data_t;
extern calibration_data_t cal;

char manufacturer[] = "LibreSolar";
char strbuf[300];

static float f32;

static uint64_t ui64;
static int64_t i64;

static uint32_t ui32;
static int32_t i32;

static uint16_t ui16;
static int16_t i16;

static bool b;

static uint32_t secret_user = 1;
static uint32_t secret_maker = 2;

int A[100];
ArrayInfo int32_array = {A, 0, 0, 0};

float B[100];
ArrayInfo float32_array = {B, 0, 0, 0};

void dummy(void);

static const data_object_t dataObjects[] = {

    // info

    TS_DATA_OBJ_STRING(0x1001, "manufacturer", manufacturer, 0,
        TS_INFO, TS_READ_ALL),

    // input data

    TS_DATA_OBJ_BOOL(0x3001, "loadEnTarget", &cal.load_enabled_target,
        TS_INPUT, TS_READ_ALL | TS_WRITE_ALL),

    TS_DATA_OBJ_BOOL(0x3002, "usbEnTarget", &cal.usb_enabled_target,
        TS_INPUT, TS_READ_ALL | TS_WRITE_ALL),

    // output data

    TS_DATA_OBJ_INT32(0x4001, "i32_output", &i32,
        TS_OUTPUT, TS_READ_ALL),

    // rpc / exec

    TS_DATA_OBJ_EXEC(0x5001, "dummy", &dummy, TS_EXEC_ALL),

    // configuration data

    TS_DATA_OBJ_UINT64(0x6001, "ui64", &ui64, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
    TS_DATA_OBJ_INT64(0x6002, "i64", &i64, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
    TS_DATA_OBJ_UINT32(0x6003, "ui32", &ui32, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
    TS_DATA_OBJ_INT32(0x6004, "i32", &i32, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
    TS_DATA_OBJ_UINT16(0x6005, "ui16", &ui16, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
    TS_DATA_OBJ_INT16(0x6006, "i16", &i16, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
    TS_DATA_OBJ_FLOAT(0x6007, "f32", &f32, 2, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
    TS_DATA_OBJ_BOOL(0x6008, "bool", &b, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),

    TS_DATA_OBJ_STRING(0x6009, "strbuf", strbuf, sizeof(strbuf),
        TS_CONF, TS_READ_ALL | TS_WRITE_ALL),

    TS_DATA_OBJ_FLOAT(0x600A, "f32_rounded", &f32, 0,
        TS_CONF, TS_READ_ALL | TS_WRITE_ALL),

    TS_DATA_OBJ_UINT32(0x7001, "secret_user", &ui32, TS_CONF, TS_READ_ALL | TS_WRITE_USER),
    TS_DATA_OBJ_UINT32(0x7002, "secret_maker", &ui32, TS_CONF, TS_READ_ALL | TS_WRITE_MAKER), 
    TS_DATA_OBJ_ARRAY(0x7003, "arrayi32", &int32_array, 0, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
    // data_obj->detail will specify the number of decimal places for float
    TS_DATA_OBJ_ARRAY(0x7004, "arrayfloat", &float32_array, 2, TS_CONF, TS_READ_ALL | TS_WRITE_ALL),
};

static const uint16_t pub_data_objects[] = {
        #if TS_64BIT_TYPES_SUPPORT
        0x6001,
        0x6002,
        #endif
        0x6003,
        0x6004,
        0x6005,
        0x6006,
        0x6007,
        0x6008,
        0x6009
};

static ts_pub_channel_t pub_channels[] = {
    { "Serial_1s", pub_data_objects, sizeof(pub_data_objects)/sizeof(uint16_t), false }
};

#endif
