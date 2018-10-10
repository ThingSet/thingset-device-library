# C library for the ThingSet Protocol

This plain C implementation is used by the LibreSolar devices' firmware.

After implementation is stable, a wrapper class for Arduino (C++) is planned.

## Text-based protocol

The following ThingSet functions are fully implemented:

- !read
- !write
- !list

Following functions are currently in progress with high priority

- !pub

In order to reduce code size, verbose status messages can be turned of using the TS_VERBOSE_STATUS_MESSAGES = 0 in ts_config.h.

## Binary protocol

The following functions are fully implemented:

- read (0x01)
- write (0x02)

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