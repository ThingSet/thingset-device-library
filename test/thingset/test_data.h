

#ifndef DATA_OBJECTS_H
#define DATA_OBJECTS_H

#include <stdint.h>
#include <stdbool.h>

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
static uint32_t secret_root = 2;

void dummy(void);

static const data_object_t dataObjects[] = {
    // info
    {0x1001, TS_INFO, TS_ACCESS_READ, TS_T_STRING, 0, (void*) manufacturer, "manufacturer"},

    // input data
    {0x3001, TS_INPUT, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_BOOL, 0, (void*) &(cal.load_enabled_target), "loadEnTarget"},
    {0x3002, TS_INPUT, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_BOOL, 0, (void*) &(cal.usb_enabled_target), "usbEnTarget"},

    // output data
    {0x4001, TS_OUTPUT, TS_ACCESS_READ, TS_T_INT32, 2, (void*) &i32, "i32_output"},

    // rpc / exec
    {0x5001, TS_EXEC, TS_ACCESS_EXEC, TS_T_BOOL, 0, (void*) &dummy, "dummy"},

    // calibration data
    {0x6001, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_UINT64,  0, (void*) &ui64, "ui64"},
    {0x6002, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_INT64,   0, (void*) &i64, "i64"},
    {0x6003, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_UINT32,  0, (void*) &ui32, "ui32"},
    {0x6004, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_INT32,   0, (void*) &i32, "i32"},
    {0x6005, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_UINT16,  0, (void*) &ui16, "ui16"},
    {0x6006, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_INT16,   0, (void*) &i16, "i16"},
    {0x6007, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_FLOAT32, 2, (void*) &f32, "f32"},
    {0x6008, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_BOOL,    0, (void*) &b, "bool"},
    {0x6009, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_STRING,  sizeof(strbuf), (void*) strbuf, "strbuf"},
    {0x600A, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE, TS_T_FLOAT32, 0, (void*) &f32, "f32_rounded"},

    {0x7001, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE_USER, TS_T_UINT32,  0, (void*) &secret_user, "secret_user"},
    {0x7002, TS_CONF, TS_ACCESS_READ | TS_ACCESS_WRITE_ROOT, TS_T_UINT32,  0, (void*) &secret_root, "secret_root"},
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

static const ts_pub_channel_t pub_channels[] = {
    { "Serial", pub_data_objects, sizeof(pub_data_objects)/sizeof(uint16_t), false }
};


#endif
