/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 */

#ifndef TESTS_H_
#define TESTS_H_

/*
 * Same as in test_data.h
 */
#define ID_ROOT     0x00
#define ID_INFO     0x18        // read-only device information (e.g. manufacturer, device ID)
#define ID_CONF     0x30        // configurable settings
#define ID_INPUT    0x60        // input data (e.g. set-points)
#define ID_OUTPUT   0x70        // output data (e.g. measurement values)
#define ID_REC      0xA0        // recorded data (history-dependent)
#define ID_CAL      0xD0        // calibration
#define ID_EXEC     0xE0        // function call
#define ID_PUB      0xF0        // publication setup
#define ID_SUB      0xF1        // subscription setup
#define ID_LOG      0x100       // access log data

#define TS_REQ_BUFFER_LEN 500
#define TS_RESP_BUFFER_LEN 500

void tests_common();
void tests_json();
void tests_cbor();

#endif /* TESTS_H_ */
