/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#include "cbor.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
int cbor_serialize_uint(uint8_t *data, uint64_t value, size_t max_len)
#else
int cbor_serialize_uint(uint8_t *data, uint32_t value, size_t max_len)
#endif
{
    if (max_len < 1) {
        return 0; // error
    }
    else if (value <= CBOR_NUM_MAX) {
        data[0] = CBOR_UINT | (uint8_t)value;
        // printf("serialize: value = %.2X <= CBOR_NUM_MAX, data: %.2X\n", (uint32_t)value,
        // data[0]);
        return 1;
    }
    else if (value <= UINT8_MAX && max_len >= 2) {
        data[0] = CBOR_UINT | CBOR_UINT8_FOLLOWS;
        data[1] = value;
        // printf("serialize: value = 0x%.2X < 0xFF, data: %.2X %.2X\n", (uint32_t)value, data[0],
        // data[1]);
        return 2;
    }
    else if (value <= UINT16_MAX && max_len >= 3) {
        data[0] = CBOR_UINT | CBOR_UINT16_FOLLOWS;
        data[1] = value >> 8;
        data[2] = value;
        // printf("serialize: value = 0x%.4X <= 0xFFFF, data: %.2X %.2X %.2X\n", (uint32_t)value,
        // data[0], data[1], data[2]);
        return 3;
    }
    else if (value <= UINT32_MAX && max_len >= 5) {
        data[0] = CBOR_UINT | CBOR_UINT32_FOLLOWS;
        data[1] = value >> 24;
        data[2] = value >> 16;
        data[3] = value >> 8;
        data[4] = value;
        // printf("serialize: value = 0x%.8X <= 0xFFFFFFFF, data: %.2X %.2X %.2X %.2X %.2X\n",
        // (uint32_t)value, data[0], data[1], data[2], data[3], data[4]);
        return 5;
    }
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    else if (max_len >= 9) {
        data[0] = CBOR_UINT | CBOR_UINT64_FOLLOWS;
        data[1] = (value >> 32) >> 24;
        data[2] = (value >> 32) >> 16;
        data[3] = (value >> 32) >> 8;
        data[4] = (value >> 32);
        data[5] = value >> 24;
        data[6] = value >> 16;
        data[7] = value >> 8;
        data[8] = value;
        return 9;
    }
#endif
    else {
        return 0;
    }
}

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
int cbor_serialize_int(uint8_t *data, int64_t value, size_t max_len)
#else
int cbor_serialize_int(uint8_t *data, int32_t value, size_t max_len)
#endif
{
    if (max_len < 1) {
        return 0;
    }

    if (value >= 0) {
        return cbor_serialize_uint(data, value, max_len);
    }
    else {
        int size = cbor_serialize_uint(data, -1 - value, max_len);
        data[0] |= CBOR_NEGINT; // set major type 1 for negative integer
        return size;
    }
}

#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
int cbor_serialize_decfrac(uint8_t *data, int32_t mantissa, int16_t exponent, size_t max_len)
{
    if (max_len < (2 + 3 + 5)) {
        return 0;
    }

    data[0] = (CBOR_TAG | CBOR_DECFRAC_ARRAY_FOLLOWS);
    data[1] = 0x82;

    int len = 2;
    len += cbor_serialize_int(&data[len], exponent, max_len - len);
    len += cbor_serialize_int(&data[len], mantissa, max_len - len);

    return len;
}
#endif

int cbor_serialize_float(uint8_t *data, float value, size_t max_len)
{
    if (max_len < 5)
        return 0;

    data[0] = CBOR_FLOAT32;

    union {
        float f;
        uint32_t ui;
    } f2ui;

    f2ui.f = value;
    data[1] = f2ui.ui >> 24;
    data[2] = f2ui.ui >> 16;
    data[3] = f2ui.ui >> 8;
    data[4] = f2ui.ui;

    return 5;
}

int cbor_serialize_bool(uint8_t *data, bool value, size_t max_len)
{
    if (max_len < 1)
        return 0;

    data[0] = value ? CBOR_TRUE : CBOR_FALSE;
    return 1;
}

int cbor_serialize_string(uint8_t *data, const char *value, size_t max_len)
{
    unsigned int len = strlen(value);

    // printf("serialize string: \"%s\", len = %d, max_len = %d\n", value, len, max_len);

    if (len <= CBOR_NUM_MAX && len + 1 <= max_len) {
        data[0] = CBOR_TEXT | (uint8_t)len;
        strcpy((char *)&data[1], value);
        return len + 1;
    }
    else if (len < UINT8_MAX && len + 2 <= max_len) {
        data[0] = CBOR_TEXT | CBOR_UINT8_FOLLOWS;
        data[1] = (uint8_t)len;
        strcpy((char *)&data[2], value);
        return len + 2;
    }
    else if (len < UINT16_MAX && len + 3 <= max_len) {
        data[0] = CBOR_TEXT | CBOR_UINT16_FOLLOWS;
        data[1] = (uint16_t)len >> 8;
        data[2] = (uint16_t)len;
        strcpy((char *)&data[3], value);
        return len + 3;
    }
    else {
        // string too long (more than 65535 characters)
        return 0;
    }
}

#if CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT
int cbor_serialize_bytes(uint8_t *data, const uint8_t *bytes, size_t num_bytes, size_t max_len)
{
    if (num_bytes <= CBOR_NUM_MAX && num_bytes + 1 <= max_len) {
        data[0] = CBOR_BYTES | (uint8_t)num_bytes;
        memcpy(&data[1], bytes, num_bytes);
        return num_bytes + 1;
    }
    else if (num_bytes < UINT8_MAX && num_bytes + 2 <= max_len) {
        data[0] = CBOR_BYTES | CBOR_UINT8_FOLLOWS;
        data[1] = (uint8_t)num_bytes;
        memcpy((char *)&data[2], bytes, num_bytes);
        return num_bytes + 2;
    }
    else if (num_bytes < UINT16_MAX && num_bytes + 3 <= max_len) {
        data[0] = CBOR_BYTES | CBOR_UINT16_FOLLOWS;
        data[1] = (uint16_t)num_bytes >> 8;
        data[2] = (uint16_t)num_bytes;
        memcpy((char *)&data[3], bytes, num_bytes);
        return num_bytes + 3;
    }
    else {
        // too many bytes (more than 65535)
        return 0;
    }
}
#endif

int _serialize_num_elements(uint8_t *data, size_t num_elements, size_t max_len)
{
    if (num_elements <= CBOR_NUM_MAX && max_len > 0) {
        data[0] |= (uint8_t)num_elements;
        return 1;
    }
    else if (num_elements < UINT8_MAX && max_len > 1) {
        data[0] |= CBOR_UINT8_FOLLOWS;
        data[1] = (uint8_t)num_elements;
        return 2;
    }
    else if (num_elements < UINT16_MAX && max_len > 1) {
        data[0] |= CBOR_UINT16_FOLLOWS;
        data[1] = (uint16_t)num_elements >> 8;
        data[2] = (uint16_t)num_elements;
        return 3;
    }
    else {
        // too many elements (more than 65535)
        return 0;
    }
}

int cbor_serialize_map(uint8_t *data, size_t num_elements, size_t max_len)
{
    data[0] = CBOR_MAP;
    return _serialize_num_elements(data, num_elements, max_len);
}

int cbor_serialize_array(uint8_t *data, size_t num_elements, size_t max_len)
{
    data[0] = CBOR_ARRAY;
    return _serialize_num_elements(data, num_elements, max_len);
}

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
int _cbor_uint_data(const uint8_t *data, uint64_t *bytes)
#else
int _cbor_uint_data(const uint8_t *data, uint32_t *bytes)
#endif
{
    uint8_t info = data[0] & CBOR_INFO_MASK;

    // printf("uint_data: %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X\n", data[0], data[1],
    // data[2], data[3], data[4], data[5], data[6], data[7], data[8]);

    if (info <= CBOR_NUM_MAX) {
        *bytes = info;
        return 1;
    }
    else if (info == CBOR_UINT8_FOLLOWS) {
        *bytes = data[1];
        return 2;
    }
    else if (info == CBOR_UINT16_FOLLOWS) {
        *bytes = data[1] << 8 | data[2];
        return 3;
    }
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    else if (info == CBOR_UINT32_FOLLOWS) {
        *(uint64_t *)bytes = ((uint64_t)data[1] << 24) | ((uint64_t)data[2] << 16)
                             | ((uint64_t)data[3] << 8) | ((uint64_t)data[4]);
        return 5;
    }
    else if (info == CBOR_UINT64_FOLLOWS) {
        *(uint64_t *)bytes = ((uint64_t)data[1] << 56) | ((uint64_t)data[2] << 48)
                             | ((uint64_t)data[3] << 40) | ((uint64_t)data[4] << 32)
                             | ((uint64_t)data[5] << 24) | ((uint64_t)data[6] << 16)
                             | ((uint64_t)data[7] << 8) | ((uint64_t)data[8]);
        return 9;
    }
#else
    else if (info == CBOR_UINT32_FOLLOWS) {
        *bytes = data[1] << 24 | data[2] << 16 | data[3] << 8 | data[4];
        return 5;
    }
#endif
    else {
        return 0;
    }
}

#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
int cbor_deserialize_uint64(const uint8_t *data, uint64_t *value)
{
    uint64_t tmp;
    int size;
    uint8_t type = data[0] & CBOR_TYPE_MASK;

    if (!value || type != CBOR_UINT)
        return 0;

    size = _cbor_uint_data(data, &tmp);
    if (size > 0 && tmp <= UINT64_MAX) {
        *value = tmp;
        return size;
    }
    return 0;
}

int cbor_deserialize_int64(const uint8_t *data, int64_t *value)
{
    uint64_t tmp;
    int size;
    uint8_t type = data[0] & CBOR_TYPE_MASK;

    if (!value || (type != CBOR_UINT && type != CBOR_NEGINT))
        return 0;

    size = _cbor_uint_data(data, &tmp);
    if (size > 0) {
        if (type == CBOR_UINT) {
            if (tmp <= INT64_MAX) {
                *value = (int64_t)tmp;
                // printf("deserialize: value = 0x%.8X <= 0xFFFFFFFF, data: %.2X %.2X %.2X %.2X"
                // " %.2X\n", (uint32_t)tmp, data[0], data[1], data[2], data[3], data[4]);
                return size;
            }
        }
        else if (type == CBOR_NEGINT) {
            // check if CBOR negint fits into C int
            // -1 - tmp >= -INT32_MAX - 1         | x (-1)
            // 1 + tmp <= INT32_MAX + 1
            if (tmp <= INT64_MAX) {
                *value = -1 - (uint64_t)tmp;
                // printf("deserialize: value = %.8X, tmp = %.8X, data: %.2X %.2X %.2X %.2X %.2X\n",
                //   *value, (uint32_t)tmp, data[0], data[1], data[2], data[3], data[4]);
                return size;
            }
        }
    }

    return 0;
}
#endif

int cbor_deserialize_uint32(const uint8_t *data, uint32_t *value)
{
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    uint64_t tmp;
#else
    uint32_t tmp;
#endif
    int size;
    uint8_t type = data[0] & CBOR_TYPE_MASK;

    // printf("deserialize: value = 0x%.8X <= 0xFFFFFFFF, data: %.2X %.2X %.2X %.2X %.2X\n",
    //   (uint32_t)value, data[0], data[1], data[2], data[3], data[4]);

    if (!value || type != CBOR_UINT)
        return 0;

    size = _cbor_uint_data(data, &tmp);
    if (size > 0 && tmp <= UINT32_MAX) {
        *value = tmp;
        return size;
    }
    return 0;
}

int cbor_deserialize_int32(const uint8_t *data, int32_t *value)
{
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
    uint64_t tmp;
#else
    uint32_t tmp;
#endif
    int size;
    uint8_t type = data[0] & CBOR_TYPE_MASK;

    if (!value || (type != CBOR_UINT && type != CBOR_NEGINT))
        return 0;

    size = _cbor_uint_data(data, &tmp);
    if (size > 0) {
        if (type == CBOR_UINT) {
            if (tmp <= INT32_MAX) {
                *value = (int32_t)tmp;
                // printf("deserialize: value = 0x%.8X <= 0xFFFFFFFF, data: %.2X %.2X %.2X %.2X"
                // " %.2X\n", (uint32_t)tmp, data[0], data[1], data[2], data[3], data[4]);
                return size;
            }
        }
        else if (type == CBOR_NEGINT) {
            // check if CBOR negint fits into C int
            // -1 - tmp >= -INT32_MAX - 1         | x (-1)
            // 1 + tmp <= INT32_MAX + 1
            if (tmp <= INT32_MAX) {
                *value = -1 - (uint32_t)tmp;
                // printf("deserialize: value = %.8X, tmp = %.8X, data: %.2X %.2X %.2X %.2X %.2X\n",
                //   *value, (uint32_t)tmp, data[0], data[1], data[2], data[3], data[4]);
                return size;
            }
        }
    }

    return 0;
}

int cbor_deserialize_uint16(const uint8_t *data, uint16_t *value)
{
    uint32_t tmp;
    int size = cbor_deserialize_uint32(data, &tmp); // also checks value for null-pointer

    if (size > 0 && tmp <= UINT16_MAX) {
        *value = tmp;
        return size;
    }
    return 0;
}

int cbor_deserialize_int16(const uint8_t *data, int16_t *value)
{
    int32_t tmp;
    int size = cbor_deserialize_int32(data, &tmp); // also checks value for null-pointer

    if (size > 0 && tmp <= INT16_MAX && tmp >= INT16_MIN) {
        *value = tmp;
        return size;
    }
    return 0;
}

int cbor_deserialize_uint8(const uint8_t *data, uint8_t *value)
{
    uint32_t tmp;
    int size = cbor_deserialize_uint32(data, &tmp); // also checks value for null-pointer

    if (size > 0 && tmp <= UINT8_MAX) {
        *value = tmp;
        return size;
    }
    return 0;
}

int cbor_deserialize_int8(const uint8_t *data, int8_t *value)
{
    int32_t tmp;
    int size = cbor_deserialize_int32(data, &tmp); // also checks value for null-pointer

    if (size > 0 && tmp <= INT8_MAX && tmp >= INT8_MIN) {
        *value = tmp;
        return size;
    }
    return 0;
}

#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
int cbor_deserialize_decfrac(const uint8_t *data, int32_t *mantissa, const int16_t exponent)
{
    int pos = 0;
    uint8_t type = data[0] & CBOR_TYPE_MASK;

    if (data[0] == (CBOR_TAG | CBOR_DECFRAC_ARRAY_FOLLOWS) && data[1] == (CBOR_ARRAY | 2U)) {
        int32_t mantissa_tmp;
        int16_t exponent_received;
        pos = 2;
        pos += cbor_deserialize_int16(&data[pos], &exponent_received);
        pos += cbor_deserialize_int32(&data[pos], &mantissa_tmp);

        for (int i = exponent_received; i < exponent; i++) {
            mantissa_tmp /= 10;
        }
        for (int i = exponent_received; i > exponent; i--) {
            mantissa_tmp *= 10;
        }
        *mantissa = mantissa_tmp;
    }
    else if (data[0] == CBOR_FLOAT32) {
        float value;
        pos = cbor_deserialize_float(&data[pos], &value);

        for (int i = 0; i < exponent; i++) {
            value /= 10.0F;
        }
        for (int i = 0; i > exponent; i--) {
            value *= 10.0F;
        }
        *mantissa = (int32_t)value;
    }
    else if (type == CBOR_UINT || type == CBOR_NEGINT) {
        int32_t value;
        pos = cbor_deserialize_int32(&data[pos], &value);

        for (int i = 0; i < exponent; i++) {
            value /= 10;
        }
        for (int i = 0; i > exponent; i--) {
            value *= 10;
        }
        *mantissa = value;
    }

    return pos;
}
#endif

int cbor_deserialize_float(const uint8_t *data, float *value)
{
    if (!value) {
        return 0;
    }

    uint8_t type = data[0] & CBOR_TYPE_MASK;
    if (type == CBOR_UINT) {
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
        uint64_t tmp;
        int len = cbor_deserialize_uint64(data, &tmp);
        *value = (float)tmp;
        return len;
#else
        uint32_t tmp;
        int len = cbor_deserialize_uint32(data, &tmp);
        *value = (float)tmp;
        return len;
#endif
    }
    else if (type == CBOR_NEGINT) {
#if CONFIG_THINGSET_64BIT_TYPES_SUPPORT
        int64_t tmp;
        int len = cbor_deserialize_int64(data, &tmp);
        *value = (float)tmp;
        return len;
#else
        int32_t tmp;
        int len = cbor_deserialize_int32(data, &tmp);
        *value = (float)tmp;
        return len;
#endif
    }
    else if (data[0] == CBOR_FLOAT32) {
        union {
            float f;
            uint32_t ui;
        } f2ui;
        f2ui.ui = data[1] << 24 | data[2] << 16 | data[3] << 8 | data[4];
        *value = f2ui.f;
        return 5;
    }
    return 0;
}

int cbor_deserialize_bool(const uint8_t *data, bool *value)
{
    if (!value || !data)
        return 0;

    if (data[0] == CBOR_TRUE) {
        *value = true;
        return 1;
    }
    else if (data[0] == CBOR_FALSE) {
        *value = false;
        return 1;
    }
    return 0;
}

int cbor_deserialize_string(const uint8_t *data, char *str, uint16_t buf_size)
{
    char *str_start;
    uint16_t str_len;

    int ret = cbor_deserialize_string_zero_copy(data, &str_start, &str_len);

    if (ret > 0 && str_len < buf_size) {
        strncpy(str, str_start, str_len);
        str[str_len] = '\0';
        return ret;
    }
    return 0;
}

int cbor_deserialize_string_zero_copy(const uint8_t *data, char **str_start, uint16_t *str_len)
{
    uint8_t type = data[0] & CBOR_TYPE_MASK;
    uint8_t info = data[0] & CBOR_INFO_MASK;

    if (!data || !str_start || !str_len || type != CBOR_TEXT) {
        return 0;
    }

    if (info <= CBOR_NUM_MAX) {
        *str_len = info;
        *str_start = (char *)&data[1];
        return *str_len + 1;
    }
    else if (info == CBOR_UINT8_FOLLOWS) {
        *str_len = data[1];
        *str_start = (char *)&data[2];
        return *str_len + 2;
    }
    else if (info == CBOR_UINT16_FOLLOWS) {
        *str_len = data[1] << 8 | data[2];
        *str_start = (char *)&data[3];
        return *str_len + 3;
    }
    return 0; // longer string not supported
}

#if CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT
int cbor_deserialize_bytes(const uint8_t *data, uint8_t *bytes, uint16_t buf_size,
                           uint16_t *num_bytes)
{
    uint8_t type = data[0] & CBOR_TYPE_MASK;
    uint8_t info = data[0] & CBOR_INFO_MASK;
    uint16_t len;

    if (!bytes || type != CBOR_BYTES)
        return 0;

    if (info <= CBOR_NUM_MAX) {
        len = info;
        if (len <= buf_size) {
            memcpy(bytes, &data[1], len);
            *num_bytes = len;
            return len + 1;
        }
    }
    else if (info == CBOR_UINT8_FOLLOWS) {
        len = data[1];
        if (len <= buf_size) {
            memcpy(bytes, &data[2], len);
            *num_bytes = len;
            return len + 2;
        }
    }
    else if (info == CBOR_UINT16_FOLLOWS) {
        len = data[1] << 8 | data[2];
        if (len <= buf_size) {
            memcpy(bytes, &data[3], len);
            *num_bytes = len;
            return len + 3;
        }
    }
    return 0; // more bytes not supported
}
#endif

// stores size of map or array in num_elements
int cbor_num_elements(const uint8_t *data, uint16_t *num_elements)
{
    uint8_t type = data[0] & CBOR_TYPE_MASK;
    uint8_t info = data[0] & CBOR_INFO_MASK;

    // printf("type: %x, info: %x\n", type, info);

    if (!num_elements)
        return 0;

    // normal type (single data element)
    if (type != CBOR_MAP && type != CBOR_ARRAY) {
        *num_elements = 1;
        return 0;
    }

    if (info <= CBOR_NUM_MAX) {
        *num_elements = info;
        return 1;
    }
    else if (info == CBOR_UINT8_FOLLOWS) {
        *num_elements = data[1];
        return 2;
    }
    else if (info == CBOR_UINT16_FOLLOWS) {
        *num_elements = data[1] << 8 | data[2];
        return 3;
    }
    return 0; // more map/array elements not supported
}

// determines the size of a cbor data item starting at given pointer
int cbor_size(const uint8_t *data)
{
    uint8_t type = data[0] & CBOR_TYPE_MASK;
    uint8_t info = data[0] & CBOR_INFO_MASK;

    if (type == CBOR_UINT || type == CBOR_NEGINT) {
        if (info <= CBOR_NUM_MAX)
            return 1;
        switch (info) {
            case CBOR_UINT8_FOLLOWS:
                return 2;
            case CBOR_UINT16_FOLLOWS:
                return 3;
            case CBOR_UINT32_FOLLOWS:
                return 5;
            case CBOR_UINT64_FOLLOWS:
                return 9;
        }
    }
    else if (type == CBOR_BYTES || type == CBOR_TEXT) {
        if (info <= CBOR_NUM_MAX) {
            return info + 1;
        }
        else {
            if (info == CBOR_UINT8_FOLLOWS)
                return 1 + data[1];
            else if (info == CBOR_UINT16_FOLLOWS)
                return 1 + (data[1] << 8 | data[2]);
            else
                return 0; // longer string / byte array not supported
        }
    }
#if CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
    else if (type == CBOR_TAG && info == CBOR_DECFRAC_ARRAY_FOLLOWS) {
        int pos = 2;
        pos += cbor_size(&data[pos]); // exponent
        pos += cbor_size(&data[pos]); // mantissa
        return pos;
    }
#endif
    else if (type == CBOR_MISC) {
        switch (data[0]) {
            case CBOR_FALSE:
            case CBOR_TRUE:
                return 1;
                break;
            case CBOR_FLOAT32:
                return 5;
                break;
            case CBOR_FLOAT64:
                return 9;
                break;
        }
    }

    return 0; // float16, arrays, maps, tagged types, etc. curently not supported
}
