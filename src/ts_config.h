/*
 * Copyright (c) 2017 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief ThingSet configuration definition.
 *
 * To be included by the specific environment implementation.
 */

#ifndef TS_CONFIG_H_
#define TS_CONFIG_H_

#ifdef THINGSET_CONFIG_HEADER
#include THINGSET_CONFIG_HEADER
#endif

/**
 * @brief ThingSet configuration definitions.
 *
 * @defgroup ts_config_priv ThingSet configuration definitions
 * @{
 */

/**
 * @def TS_CONFIG_CORE
 *
 * @brief Configure ThingSet core support.
 *
 * Core support is a minimum capabilities configuration for ThingSet. Use this for very resource
 * constrained Things.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_CORE to set
 *       @ref TS_CONFIG_CORE.
 */
#if !defined(TS_CONFIG_CORE) && !defined(CONFIG_THINGSET_CORE)
#define TS_CONFIG_CORE 0
#elif !defined(TS_CONFIG_CORE)
#define TS_CONFIG_CORE CONFIG_THINGSET_CORE
#endif

/**
 * @def TS_CONFIG_CORE_LOCID
 *
 * @brief Configure core variant ThingSet local context identifier.
 *
 * Core support works on a single core variant ThingSet local context. The local context identifier
 * used for the single core context is given by @ref TS_CONFIG_CORE_LOCID.
 *
 * - default: 0
 *
 * @note Kconfig TS_CONFIG_CORE_LOCID build systems (or others) may use
 *       @ref CONFIG_THINGSET_CORE_LOCID to set @ref TS_CONFIG_CORE_LOCID.
 */
#if !TS_CONFIG_CORE
#define TS_CONFIG_CORE_LOCID 99
#elif !defined(TS_CONFIG_CORE_LOCID) && !defined(CONFIG_THINGSET_CORE_LOCID)
#define TS_CONFIG_CORE_LOCID 0
#elif !defined(TS_CONFIG_CORE_LOCID)
#define TS_CONFIG_CORE_LOCID CONFIG_THINGSET_CORE_LOCID
#endif

/**
 * @def TS_CONFIG_COM
 *
 * @brief Configure ThingSet communication framework support.
 *
 * Communication support provides a framework for communication for ThingSet. Use this (instead of
 * @p TS_CONFIG_CORE) for Things that need more elaborated communication capabilities. As a
 * drawback this configuration needs more resources.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_COM to set
 *       @ref TS_CONFIG_COM.
 */
#if !defined(TS_CONFIG_COM) && !defined(CONFIG_THINGSET_COM)
#define TS_CONFIG_COM 0
#elif !defined(TS_CONFIG_COM)
#define TS_CONFIG_COM CONFIG_THINGSET_COM
#endif

/**
 * @def TS_CONFIG_CPP_LEGACY
 *
 * @brief Configure legacy C++ interface support.
 *
 * This option enables the legacy C++ interface of the
 * ThingSet protocol library. Enable if your C++ code uses
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_CPP_LEGACY to set
 *       @ref TS_CONFIG_CPP_LEGACY.
 */
#if defined(__cplusplus) && !defined(TS_CONFIG_CPP_LEGACY) && !defined(CONFIG_THINGSET_CPP_LEGACY)
#define TS_CONFIG_CPP_LEGACY 0
#elif defined(__cplusplus) && !defined(TS_CONFIG_CPP_LEGACY)
#define TS_CONFIG_CPP_LEGACY CONFIG_THINGSET_CPP_LEGACY
#endif

/**
 * @def TS_CONFIG_NUM_JSON_TOKENS
 *
 * @brief Configure the maximum number of expected JSON tokens.
 *
 * JSON tokens i.e. arrays, map keys, values, primitives, etc.
 *
 * ThingSet throws an error if maximum number of tokens is reached in a
 * request or response.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_NUM_JSON_TOKENS to set
 *       @ref TS_CONFIG_NUM_JSON_TOKENS.
 */
#if !defined(TS_CONFIG_NUM_JSON_TOKENS) && !defined(CONFIG_THINGSET_NUM_JSON_TOKENS)
#define TS_CONFIG_NUM_JSON_TOKENS 50
#elif !defined(TS_CONFIG_NUM_JSON_TOKENS)
#define TS_CONFIG_NUM_JSON_TOKENS CONFIG_THINGSET_NUM_JSON_TOKENS
#endif

/**
 * @def TS_CONFIG_VERBOSE_STATUS_MESSAGES
 *
 * @brief Configure verbose status messaged.
 *
 * If verbose status messages are switched on, a response in text-based mode
 * contains not only the status code, but also a message.
 *
 * @note Kconfig based build systems (or others) may use
 *       @ref CONFIG_THINGSET_VERBOSE_STATUS_MESSAGES to set @ref TS_CONFIG_VERBOSE_STATUS_MESSAGES.
 */
#if !defined(TS_CONFIG_VERBOSE_STATUS_MESSAGES) && !defined(CONFIG_THINGSET_VERBOSE_STATUS_MESSAGES)
#define TS_CONFIG_VERBOSE_STATUS_MESSAGES 1
#elif !defined(TS_CONFIG_VERBOSE_STATUS_MESSAGES)
#define TS_CONFIG_VERBOSE_STATUS_MESSAGES CONFIG_THINGSET_VERBOSE_STATUS_MESSAGES
#endif

/**
 * @def TS_CONFIG_64BIT_TYPES_SUPPORT
 *
 * @brief Configure 64 bit variable types support.
 *
 * Switch on support for 64 bit variable types (uint64_t, int64_t, double)
 *
 * This should be disabled for most 8-bit microcontrollers to increase
 * performance
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_64BIT_TYPES_SUPPORT
 *       to set @ref TS_CONFIG_64BIT_TYPES_SUPPORT.
 */
#if !defined(TS_CONFIG_64BIT_TYPES_SUPPORT) && !defined(CONFIG_THINGSET_64BIT_TYPES_SUPPORT)
#define TS_CONFIG_64BIT_TYPES_SUPPORT 0        // default: no support
#elif !defined(TS_CONFIG_64BIT_TYPES_SUPPORT)
#define TS_CONFIG_64BIT_TYPES_SUPPORT CONFIG_THINGSET_64BIT_TYPES_SUPPORT
#endif

/**
 * @def TS_CONFIG_DECFRAC_TYPE_SUPPORT
 *
 * @brief Configure CBOR decimal fraction data type support.
 *
 * Switch on support for CBOR decimal fraction data type which stores a decimal mantissa
 * and a constant decimal exponent. This allows to use e.g. millivolts internally instead
 * of floating point numbers, while still communicating the SI base unit (volts).
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
 *       to set @ref TS_CONFIG_DECFRAC_TYPE_SUPPORT.
 */
#if !defined(TS_CONFIG_DECFRAC_TYPE_SUPPORT) && !defined(CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT)
#define TS_CONFIG_DECFRAC_TYPE_SUPPORT 0        // default: no support
#elif !defined(TS_CONFIG_DECFRAC_TYPE_SUPPORT)
#define TS_CONFIG_DECFRAC_TYPE_SUPPORT CONFIG_THINGSET_DECFRAC_TYPE_SUPPORT
#endif

/**
 * @def TS_CONFIG_BYTE_STRING_TYPE_SUPPORT
 *
 * @brief Configure CBOR byte strings support.
 *
 * Switch on support for CBOR byte strings, which can store any sort of binary data and
 * can be used e.g. for firmware upgrades. Byte strings are not supported by JSON.
 *
 * @note Kconfig based build systems (or others) may use
 *       @ref CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT to set
 *       @ref TS_CONFIG_BYTE_STRING_TYPE_SUPPORT.
 */
#if !defined(TS_CONFIG_BYTE_STRING_TYPE_SUPPORT) && !defined(CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT)
#define TS_CONFIG_BYTE_STRING_TYPE_SUPPORT 0        // default: no support
#elif !defined(TS_CONFIG_BYTE_STRING_TYPE_SUPPORT)
#define TS_CONFIG_BYTE_STRING_TYPE_SUPPORT CONFIG_THINGSET_BYTE_STRING_TYPE_SUPPORT
#endif

/**
 * @def TS_CONFIG_LOCAL_COUNT
 *
 * @brief Configure the number of local contexts.
 *
 * ThingSet holds one local contexts' objects databases per local context. Configure the total
 * number of local contexts and their object databases that a device has.
 *
 * A device may host several local contexts.
 * - default is 1.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_LOCAL_COUNT to set
 *       @ref TS_CONFIG_LOCAL_COUNT.
 */
#if !defined(TS_CONFIG_LOCAL_COUNT) && !defined(CONFIG_THINGSET_LOCAL_COUNT)
#define TS_CONFIG_LOCAL_COUNT 1
#elif !defined(TS_CONFIG_LOCAL_COUNT)
#define TS_CONFIG_LOCAL_COUNT CONFIG_THINGSET_LOCAL_COUNT
#endif

/**
 * @def TS_CONFIG_REMOTE_OBJECT_COUNT
 *
 * @brief Configure the number of remote objects that may be handled by the ThingSet pool.
 *
 * ThingSet holds a pool for remote objects. Configure the total number of remote objects that a
 * device may handle.
 *
 * ThingSet core context does not need any of these. A ThingSet communication context should have
 * more depending on the application need.
 * - default is 16.
 *
 * @note Kconfig based build systems (or others) may use @ref TS_CONFIG_REMOTE_OBJECT_COUNT to set
 *       @ref TS_CONFIG_REMOTE_OBJECT_COUNT.
 */
#if !TS_CONFIG_COM && !defined(TS_CONFIG_REMOTE_OBJECT_COUNT) && \
    !defined(CONFIG_THINGSET_REMOTE_OBJECT_COUNT)
#define TS_CONFIG_REMOTE_OBJECT_COUNT 0
#elif !defined(TS_CONFIG_REMOTE_OBJECT_COUNT)
#if !defined(CONFIG_THINGSET_REMOTE_OBJECT_COUNT)
#define TS_CONFIG_REMOTE_OBJECT_COUNT 16
#else
#define TS_CONFIG_REMOTE_OBJECT_COUNT CONFIG_THINGSET_REMOTE_OBJECT_COUNT
#endif
#endif

/**
 * @def TS_CONFIG_REMOTE_COUNT
 *
 * @brief Configure the number of remote contexts that remote objects may live in.
 *
 * ThingSet holds a pool for the remote contexts' objects databases. Configure the total number
 * of remote contexts' object databases that a device may handle.
 *
 * ThingSet core context does not need any of these. A ThingSet communication context should have
 * more depending on the application need.
 * - default is 4.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_REMOTE_COUNT to set
 *       @ref TS_CONFIG_REMOTE_COUNT.
 */
#if !TS_CONFIG_COM && !defined(TS_CONFIG_REMOTE_COUNT) && !defined(CONFIG_THINGSET_REMOTE_COUNT)
#define TS_CONFIG_REMOTE_COUNT 0
#elif !defined(TS_CONFIG_REMOTE_COUNT)
#if !defined(CONFIG_THINGSET_REMOTE_COUNT)
#define TS_CONFIG_REMOTE_COUNT 4
#else
#define TS_CONFIG_REMOTE_COUNT CONFIG_THINGSET_REMOTE_COUNT
#endif
#endif

/**
 * @def TS_CONFIG_PORT_COUNT
 *
 * @brief Configure the number of ThingSet ports of a device.
 *
 * ThingSet holds a table of all the ThingSet ports of a device. Configure the total number
 * of ThingSet ports that a device handles.
 *
 * ThingSet core context does not need any of these. A ThingSet communication context should have
 * more depending on the number of interfaces used for ThingSet communication.
 * - default is 1.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_PORT_COUNT to set
 *       @ref TS_CONFIG_PORT_COUNT.
 */
#if !TS_CONFIG_COM && !defined(TS_CONFIG_PORT_COUNT) && !defined(CONFIG_THINGSET_PORT_COUNT)
#define TS_CONFIG_PORT_COUNT 0
#elif !defined(TS_CONFIG_PORT_COUNT)
#if !defined(CONFIG_THINGSET_PORT_COUNT)
#define TS_CONFIG_PORT_COUNT 1
#else
#define TS_CONFIG_PORT_COUNT CONFIG_THINGSET_PORT_COUNT
#endif
#endif

/**
 * @def TS_CONFIG_BUF_COUNT
 *
 * @brief Configure number of buffers in the ThingSet communication buffer pool.
 *
 * ThingSet core context needs two buffers. A ThingSet communication context should have more
 * - default is 16.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_COM_BUF_COUNT to set
 *       @ref TS_CONFIG_BUF_COUNT.
 */
#if !defined(TS_CONFIG_BUF_COUNT) && !defined(CONFIG_THINGSET_COM_BUF_COUNT)
#if TS_CONFIG_CORE && !TS_CONFIG_COM
#define TS_CONFIG_BUF_COUNT 2
#else
#define TS_CONFIG_BUF_COUNT 16
#endif
#elif !defined(TS_CONFIG_BUF_COUNT)
#define TS_CONFIG_BUF_COUNT CONFIG_THINGSET_COM_BUF_COUNT
#endif

/**
 * @def TS_CONFIG_BUF_DATA_SIZE
 *
 * @brief Configure data block size for ThingSet communication buffers.
 *
 * This is the total size of all buffers' data. Each active buffer takes it's share from this
 * total amount.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_COM_BUF_DATA_SIZE
 *       to set @ref TS_CONFIG_BUF_DATA_SIZE.
 */
#if !defined(TS_CONFIG_BUF_DATA_SIZE) && !defined(CONFIG_THINGSET_COM_BUF_DATA_SIZE)
#if TS_CONFIG_CORE && !TS_CONFIG_COM
#define TS_CONFIG_BUF_DATA_SIZE 512
#else
#define TS_CONFIG_BUF_DATA_SIZE 1024
#endif
#elif !defined(TS_CONFIG_BUF_DATA_SIZE)
#define TS_CONFIG_BUF_DATA_SIZE CONFIG_THINGSET_COM_BUF_DATA_SIZE
#endif

/**
 * @def TS_CONFIG_NODETABLE_SIZE
 *
 * @brief Configure maximum number of nodes a context's node table can hold.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_NODETABLE_SIZE to set
 *       @ref TS_CONFIG_NODETABLE_SIZE.
 */
#if !defined(TS_CONFIG_NODETABLE_SIZE) && !defined(CONFIG_THINGSET_NODETABLE_SIZE)
#define TS_CONFIG_NODETABLE_SIZE 3
#elif !defined(TS_CONFIG_NODETABLE_SIZE)
#define TS_CONFIG_NODETABLE_SIZE CONFIG_THINGSET_NODETABLE_SIZE
#endif

/**
 * @def TS_CONFIG_LOG
 *
 * @brief Configure ThingSet logging support.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_LOG to set
 *       @ref TS_CONFIG_LOG.
 */
#if !defined(TS_CONFIG_LOG) && !defined(CONFIG_THINGSET_LOG)
#define TS_CONFIG_LOG 0
#elif !defined(TS_CONFIG_LOG)
#define TS_CONFIG_LOG CONFIG_THINGSET_LOG
#endif

/**
 * @def TS_CONFIG_LOG_LEVEL
 *
 * @brief Configure ThingSet log level.
 *
 * The log levels are:
 * - 0: Off
 * - 1: Error
 * - 2: Warning
 * - 3: Info
 * - 4: Debug
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_LOG_LEVEL to set
 *       @ref TS_CONFIG_LOG_LEVEL.
 */
#if !defined(TS_CONFIG_LOG_LEVEL) && !defined(CONFIG_THINGSET_LOG_LEVEL)
#define TS_CONFIG_LOG_LEVEL 0
#elif !defined(TS_CONFIG_LOG_LEVEL)
#define TS_CONFIG_LOG_LEVEL CONFIG_THINGSET_LOG_LEVEL
#endif

/**
 * @def TS_CONFIG_UNIT_TEST
 *
 * @brief Configure ThingSet unit test support.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_UNIT_TEST to set
 *       @ref TS_CONFIG_UNIT_TEST.
 */
#if !defined(TS_CONFIG_UNIT_TEST) && !defined(CONFIG_THINGSET_UNIT_TEST)
#define TS_CONFIG_UNIT_TEST 0
#elif !defined(TS_CONFIG_UNIT_TEST)
#define TS_CONFIG_UNIT_TEST CONFIG_THINGSET_UNIT_TEST
#endif

/**
 * @def TS_CONFIG_ASSERT
 *
 * @brief Configure ThingSet assert support.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_ASSERT to set
 *       @ref TS_CONFIG_ASSERT.
 */
#if !defined(TS_CONFIG_ASSERT) && !defined(CONFIG_THINGSET_ASSERT)
#define TS_CONFIG_ASSERT 0
#elif !defined(TS_CONFIG_ASSERT)
#define TS_CONFIG_ASSERT CONFIG_THINGSET_ASSERT
#endif

/**
 * @def TS_CONFIG_SHELL
 *
 * @brief Configure ThingSet shell application.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_SHELL to set
 *       @ref TS_CONFIG_SHELL.
 */
#if !defined(TS_CONFIG_SHELL) && !defined(CONFIG_THINGSET_SHELL)
#define TS_CONFIG_SHELL 0
#elif !defined(TS_CONFIG_SHELL)
#define TS_CONFIG_SHELL CONFIG_THINGSET_SHELL
#endif

/**
 * @def TS_CONFIG_SHELL_NAME
 *
 * @brief Configure ThingSet shell application name.
 *
 * The name the shell application shall use at the local context.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_SHELL_NAME to set
 *       @ref TS_CONFIG_SHELL_NAME.
 */
#if !defined(TS_CONFIG_SHELL_NAME) && !defined(CONFIG_THINGSET_SHELL_NAME)
#define TS_CONFIG_SHELL_NAME "shell"
#elif !defined(TS_CONFIG_SHELL_NAME)
#define TS_CONFIG_SHELL_NAME CONFIG_THINGSET_SHELL_NAME
#endif

/**
 * @def TS_CONFIG_SHELL_LOCID
 *
 * @brief Configure ThingSet shell application local context ID.
 *
 * The local context the shell applicationm shall be attached to.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_SHELL_LOCID to set
 *       @ref TS_CONFIG_SHELL_LOCID.
 */
#if !defined(TS_CONFIG_SHELL_LOCID) && !defined(CONFIG_THINGSET_SHELL_LOCID)
#define TS_CONFIG_SHELL_LOCID 0
#elif !defined(TS_CONFIG_SHELL_LOCID)
#define TS_CONFIG_SHELL_LOCID CONFIG_THINGSET_SHELL_LOCID
#endif

/**
 * @def TS_CONFIG_SHELL_PORTID
 *
 * @brief Configure ThingSet shell application port ID.
 *
 * The ID of the port the shell shall communicate to the local context.
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_SHELL_PORTID to set
 *       @ref TS_CONFIG_SHELL_PORTID.
 */
#if !defined(TS_CONFIG_SHELL_PORTID) && !defined(CONFIG_THINGSET_SHELL_PORTID)
#define TS_CONFIG_SHELL_PORTID 0
#elif !defined(TS_CONFIG_SHELL_PORTID)
#define TS_CONFIG_SHELL_PORTID CONFIG_THINGSET_SHELL_PORTID
#endif

/**
 * @def TS_CONFIG_SHELL_MEM_SIZE
 *
 * @brief Configure size of dynamic memory the ThingSet shell may use.
 *
 * The total size of memory the ThingSet shell may allocate by ts_shell_allocate() and free by
 * ts_shell_free().
 *
 * @note Kconfig based build systems (or others) may use @ref CONFIG_THINGSET_SHELL_MEM_SIZE to set
 *       @ref TS_CONFIG_SHELL_MEM_SIZE.
 */
#if !defined(TS_CONFIG_SHELL_MEM_SIZE) && !defined(CONFIG_THINGSET_SHELL_MEM_SIZE)
#define TS_CONFIG_SHELL_MEM_SIZE 0
#elif !defined(TS_CONFIG_SHELL_MEM_SIZE)
#define TS_CONFIG_SHELL_MEM_SIZE CONFIG_THINGSET_SHELL_MEM_SIZE
#endif

/**
 * @} <!-- ts_config_priv -->
 */

#endif /* TS_CONFIG_H_ */
