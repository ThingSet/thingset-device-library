# C++ library for the ThingSet Protocol

![Travis CI build badge](https://travis-ci.com/ThingSet/thingset-cpp.svg?branch=master)

This C++ implementation is used by the LibreSolar devices' firmware.

## Text-based protocol

The following ThingSet functions are fully implemented:

- Data abject access functions for all categories (!info, !conf, ...)
- Execution of functions (!exec)
- Sending of publication messages (# {...})
- Authentication (!auth) with 3 different levels
    - All: no password
    - User password: Can be used for battery config, etc.
    - Maker password: Admin/root password for manufacturer, e.g. for calibration settings. Maker authorization level also has all rights of "all" and "user" levels.
- Publication message (!pub): List and enable/disable channels implemented, changing of channel data objects per channel still to be done.

In order to reduce code size, verbose status messages can be turned off using the TS_VERBOSE_STATUS_MESSAGES = 0 in ts_config.h.

## Binary protocol

The following functions are fully implemented:

- Data abject access functions for all categories (0x01, 0x02, ...)
- Execution of functions (0x0B)
- Sending of publication messages (0x1F)

For an efficient implementation, only the most important CBOR data types will be supported:

- Unsigned int up to 64 bit
- Negative int up to 64 bit
- UTF8 strings of up to 2^16-1 bytes
- Binary data of up to 2^16-1 bytes
- Float 32 and 64 bit
- Simple values true and false

Currently, following data types are still missing in the implementation.

- Binary data (only CBOR format)  of up to 2^16-1 bytes
- Float 64 (double)

It is possible to enable or disable 64 bit data types to decrease code size using the TS_64BIT_TYPES_SUPPORT flag in ts_config.h.

## Unit testing

Using Visual Studio Code with platformio extension, just press the test button to run the unit tests.

## Remarks

This implemntation uses the very lightweight JSON parser [JSMN](https://github.com/zserge/jsmn).