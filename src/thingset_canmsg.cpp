/* ThingSet protocol client library
 * Copyright (c) 2017-2019 Martin Jäger (www.libre.solar)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ts_config.h"
#include "thingset.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>

#define CAN_TS_T_TRUE 61
#define CAN_TS_T_FALSE 60
#define CAN_TS_T_POS_INT32  6
#define CAN_TS_T_NEG_INT32  7
#define CAN_TS_T_FLOAT32 30
#define CAN_TS_T_DECFRAC 36


/**
 * @returns -1 if the object could not be encoded successfully, message length otherwise. msg_len 0 is valid, just the id is transmitted
 * in this case
 * 
 */
int ThingSet::encode_msg_can(const data_object_t& data_obj, uint8_t can_node_id, unsigned int& msg_id, uint8_t (&msg_data)[8])
{    
    int msg_len = -1;

    union float2bytes { float f; char b[4]; } f2b;     // for conversion of float to single bytes

    const int msg_priority = 6;

    msg_id = msg_priority << 26
        | (1U << 24) | (1U << 25)   // identify as publication message
        | data_obj.id << 8
        | can_node_id;

    // first byte: TinyTP-header or data type for single frame message
    // currently: no multi-frame and no timestamp
    msg_data[0] = 0x00;

    int32_t value;
    uint32_t value_abs;

    // CBOR byte order: most significant byte first
    switch (data_obj.type) {
        case TS_T_FLOAT32:
            msg_len = 5;
            msg_data[0] |= CAN_TS_T_FLOAT32;
            f2b.f = *((float*)data_obj.data);
            msg_data[1] = f2b.b[3];
            msg_data[2] = f2b.b[2];
            msg_data[3] = f2b.b[1];
            msg_data[4] = f2b.b[0];
            break;
        case TS_T_INT32:
            if (data_obj.detail == 0) {
                    msg_len = 5;
                    value = *((int*)data_obj.data);
                    if (value >= 0) {
                        msg_data[0] |= CAN_TS_T_POS_INT32;
                        value_abs = abs(value);
                    }
                    else {
                        msg_data[0] |= CAN_TS_T_NEG_INT32;
                        value_abs = abs(value - 1);         // 0 is always pos integer
                    }
                    msg_data[1] = value_abs >> 24;
                    msg_data[2] = value_abs >> 16;
                    msg_data[3] = value_abs >> 8;
                    msg_data[4] = value_abs;
            }
            else {
                msg_len = 8;
                msg_data[0] |= CAN_TS_T_DECFRAC;
                msg_data[1] = 0x82;     // array of length 2

                // detail in single byte value, valid: -24 ... 23
                uint8_t exponent_abs = abs(data_obj.detail);
                if (data_obj.detail >= 0 && data_obj.detail <= 23) {
                    msg_data[2] = exponent_abs;
                }
                else if (data_obj.detail < 0 && data_obj.detail > -24) {
                    msg_data[2] = (exponent_abs - 1) | 0x20;      // negative uint8
                }

                // value as positive or negative uint32
                value = *((int*)data_obj.data);
                if (value >= 0) {
                    msg_data[3] = 0x1a;     // positive int32
                    value_abs = abs(value);
                }
                else {
                    msg_data[3] = 0x3a;     // negative int32
                    value_abs = abs(value - 1);         // 0 is always pos integer
                }
                msg_data[4] = value_abs >> 24;
                msg_data[5] = value_abs >> 16;
                msg_data[6] = value_abs >> 8;
                msg_data[7] = value_abs;
            }
            break;
        case TS_T_BOOL:
            msg_len = 1;
            if (*((bool*)data_obj.data) == true) {
                msg_data[0] = CAN_TS_T_TRUE;     // simple type: true
            }
            else {
                msg_data[0] = CAN_TS_T_FALSE;     // simple type: false
            }
            break;
        default:
            break;
    }
    return msg_len;
}
