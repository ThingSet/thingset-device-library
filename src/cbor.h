/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 */

#ifndef CBOR_H_
#define CBOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_config.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CBOR_TYPE_MASK          0xE0    /**< most-significant 3 bits */
#define CBOR_INFO_MASK          0x1F    /**< least-significant 5 bits */

#define CBOR_UINT       0x00            /**< Major type 0: Unsigned integers */
#define CBOR_NEGINT     0x20            /**< Major type 1: Negative integers */
#define CBOR_BYTES      0x40            /**< Major type 2: Bytes (binary) */
#define CBOR_TEXT       0x60            /**< Major type 3: Text (UTF-8) */
#define CBOR_ARRAY      0x80            /**< Major type 4: Array */
#define CBOR_MAP        0xA0            /**< Major type 5: Map / Object */
#define CBOR_TAG        0xC0            /**< Major type 6: Tag (for extension) */
#define CBOR_MISC       0xE0            /**< Major type 7: Simple values and float */

#define CBOR_NUM_MAX            23      /**< Maximum number that can be directly encoded */

#define CBOR_UINT8_FOLLOWS      24      /**< Length specifier uint8 (0x18) */
#define CBOR_UINT16_FOLLOWS     25      /**< Length specifier uint16 (0x19) */
#define CBOR_UINT32_FOLLOWS     26      /**< Length specifier uint32 (0x1a) */
#define CBOR_UINT64_FOLLOWS     27      /**< Length specifier uint64 (0x1b) */

#define CBOR_VAR_FOLLOWS        31      /**< Indefinite length specifier (0x1f) */

/* Major type 6: Tags */
#define CBOR_DATETIME_STRING_FOLLOWS        0   /**< Datetime string */
#define CBOR_DATETIME_EPOCH_FOLLOWS         1   /**< Datetime epoch */
#define CBOR_DECFRAC_ARRAY_FOLLOWS          4   /**< Decimal fraction */

/* Major type 7: Simple values and float */
#define CBOR_FALSE      (CBOR_MISC | 20)   /**< Simple value: false */
#define CBOR_TRUE       (CBOR_MISC | 21)   /**< Simple value: true */
#define CBOR_NULL       (CBOR_MISC | 22)   /**< Simple value: null */
#define CBOR_UNDEFINED  (CBOR_MISC | 23)   /**< Simple value: undefined */
#define CBOR_SIMPLE     (CBOR_MISC | 24)   /**< Simple value: uint8 follows */
#define CBOR_FLOAT16    (CBOR_MISC | 25)   /**< Half-precision float follows */
#define CBOR_FLOAT32    (CBOR_MISC | 26)   /**< Single-precision float follows */
#define CBOR_FLOAT64    (CBOR_MISC | 27)   /**< Double-precision float follows */
#define CBOR_BREAK      (CBOR_MISC | 31)   /**< Simple value: break */

/**
 * Serialize unsigned integer value
 *
 * @param data Buffer where CBOR data shall be stored
 * @param value Variable containing value to be serialized
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns Number of bytes added to buffer or 0 in case of error
 */
#if TS_64BIT_TYPES_SUPPORT
int cbor_serialize_uint(uint8_t *data, uint64_t value, size_t max_len);
#else
int cbor_serialize_uint(uint8_t *data, uint32_t value, size_t max_len);
#endif

/**
 * Serialize signed integer value
 *
 * @param data Buffer where CBOR data shall be stored
 * @param value Variable containing value to be serialized
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns Number of bytes added to buffer or 0 in case of error
 */
#if TS_64BIT_TYPES_SUPPORT
int cbor_serialize_int(uint8_t *data, int64_t value, size_t max_len);
#else
int cbor_serialize_int(uint8_t *data, int32_t value, size_t max_len);
#endif

/**
 * Serialize decimal fraction (e.g. 1234 * 10^3)
 *
 * @param data Buffer where CBOR data shall be stored
 * @param mantissa Mantissa of the value to be serialized
 * @param exponent Exponent of the value to be serialized
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns Number of bytes added to buffer or 0 in case of error
 */
int cbor_serialize_decfrac(uint8_t *data, int32_t mantissa, int16_t exponent, size_t max_len);

/**
 * Serialize 32-bit float
 *
 * @param data Buffer where CBOR data shall be stored
 * @param value Variable containing value to be serialized
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns Number of bytes added to buffer or 0 in case of error
 */
int cbor_serialize_float(uint8_t *data, float value, size_t max_len);

/**
 * Serialize boolean
 *
 * @param data Buffer where CBOR data shall be stored
 * @param value Variable containing value to be serialized
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns Number of bytes added to buffer or 0 in case of error
 */
int cbor_serialize_bool(uint8_t *data, bool value, size_t max_len);

/**
 * Serialize string
 *
 * @param data Buffer where CBOR data shall be stored
 * @param value Pointer to string that should be be serialized
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns Number of bytes added to buffer or 0 in case of error
 */
int cbor_serialize_string(uint8_t *data, const char *value, size_t max_len);

/**
 * Serialize bytes
 *
 * @param data Buffer where CBOR data shall be stored
 * @param bytes Pointer to string that should be be serialized
 * @param num_bytes Number of bytes from the buffer to be serialized
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns Number of bytes added to buffer or 0 in case of error
 */
int cbor_serialize_bytes(uint8_t *data, const uint8_t *bytes, size_t num_bytes, size_t max_len);

/**
 * Serialize the header (length field) of an array
 *
 * Actual elements of the array have to be serialized afterwards
 *
 * @param data Buffer where CBOR data shall be stored
 * @param num_elements Number of elements in the array
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns Number of bytes added to buffer or 0 in case of error
 */
int cbor_serialize_array(uint8_t *data, size_t num_elements, size_t max_len);

/**
 * Serialize the header (length field) of a map (equivalent to JSON object)
 *
 * Actual elements of the map have to be serialized afterwards
 *
 * @param data Buffer where CBOR data shall be stored
 * @param num_elements Number of elements in the array
 * @param max_len Maximum remaining space in buffer (i.e. max length of serialized data)
 *
 * @returns number of bytes added to buffer or 0 in case of error
 */
int cbor_serialize_map(uint8_t *data, size_t num_elements, size_t max_len);

/**
 * Deserialization (CBOR data to C values)
 */

/**
 * Deserialize 64-bit unsigned integer
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_uint64(const uint8_t *data, uint64_t *value);

/**
 * Deserialize 64-bit signed integer
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_int64(const uint8_t *data, int64_t *value);

/**
 * Deserialize 32-bit unsigned integer
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_uint32(const uint8_t *data, uint32_t *value);

/**
 * Deserialize 32-bit signed integer
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_int32(const uint8_t *data, int32_t *value);

/**
 * Deserialize 16-bit unsigned integer
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_uint16(const uint8_t *data, uint16_t *value);

/**
 * Deserialize 16-bit signed integer
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_int16(const uint8_t *data, int16_t *value);

/**
 * Deserialize 8-bit unsigned integer
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_uint8(const uint8_t *data, uint8_t *value);

/**
 * Deserialize 8-bit signed integer
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_int8(const uint8_t *data, int8_t *value);

/**
 * Deserialize decimal fraction type
 *
 * The exponent is fixed, so the mantissa is multiplied to match the exponent
 *
 * @param data Buffer containing CBOR data with matching type
 * @param mantissa Pointer to the variable where the mantissa should be stored
 * @param exponent Exponent of internally used variable in C
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_decfrac(const uint8_t *data, int32_t *mantissa, const int16_t exponent);

/**
 * Deserialize 32-bit float
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_float(const uint8_t *data, float *value);

/**
 * Deserialize bool
 *
 * @param data Buffer containing CBOR data with matching type
 * @param value Pointer to the variable where the value should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_deserialize_bool(const uint8_t *data, bool *value);

/**
 * Deserialize string
 *
 * @param data Buffer containing CBOR data with matching type
 * @param str Pointer to the buffer where the string should be stored
 * @param buf_size Size of the string buffer including null-termination
 *
 * @returns Number of bytes read from data buffer or 0 in case of error
 */
int cbor_deserialize_string(const uint8_t *data, char *str, uint16_t buf_size);

/**
 * Deserialize string with zero-copy
 *
 * @param data Buffer containing CBOR data with matching type
 * @param str_start Pointer to store start of string
 * @param str_len Pointer to store length of string in the buffer EXCLUDING null-termination
 *
 * @returns Number of bytes read from data buffer or 0 in case of error
 */
int cbor_deserialize_string_zero_copy(const uint8_t *data, char **str_start, uint16_t *str_len);

/**
 * Deserialize bytes
 *
 * @param data Buffer containing CBOR data with matching type
 * @param bytes Pointer to the buffer where the data should be stored
 * @param buf_size Size of the buffer
 * @param num_bytes Pointer to a variable to store the actual number of bytes written to buffer
 *
 * @returns Number of bytes read from data buffer or 0 in case of error
 */
int cbor_deserialize_bytes(const uint8_t *data, uint8_t *bytes, uint16_t buf_size,
                           uint16_t *num_bytes);

/**
 * Determine the number of elements in a map or an array
 *
 * @param data Buffer containing CBOR data with matching type
 * @param num_elements Pointer to the variable where the result should be stored
 *
 * @returns Number of bytes read from buffer or 0 in case of error
 */
int cbor_num_elements(const uint8_t *data, uint16_t *num_elements);

/**
 * Determine the size of the cbor data item
 *
 * @param data Pointer for starting point of data item
 *
 * @returns Size in bytes
 */
int cbor_size(const uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* CBOR_H_ */
