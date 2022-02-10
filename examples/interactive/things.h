/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2022 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "../../src/thingset.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Categories / first layer object IDs */
#define ID_ROOT     TS_ID_ROOT
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

extern char manufacturer[];
extern bool pub_report_enable;
extern uint16_t pub_report_interval;
extern bool pub_info_enable;

#ifdef __cplusplus
}
#endif
