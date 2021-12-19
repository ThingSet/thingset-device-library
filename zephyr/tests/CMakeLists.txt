# Copyright (c) 2021 Bobby Noelte.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

# Get base directory.
# Assumption: this file is within the zephyr/tests subdir of the thingset library.
get_filename_component(THINGSET_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
message(STATUS "THINGSET_BASE: ${THINGSET_BASE}")

# Search for Zepyhr installed side by side to thingset device library.
# This is a fallback in case ZEPHYR_BASE is not given.
set(zephyr_base $ENV{ZEPHYR_BASE})
get_filename_component(MODULES_BASE "${THINGSET_BASE}/.." ABSOLUTE)
FILE(GLOB modules LIST_DIRECTORIES true "${MODULES_BASE}/*")
foreach(module ${modules})
    set(full_path ${module}/Kconfig.zephyr)
    if(EXISTS ${full_path})
        set(zephyr_base "${module}")
        set(ENV{ZEPHYR_BASE} "${zephyr_base}")
        break()
    endif()
endforeach()
message(STATUS "ZEPHYR_BASE: ${zephyr_base}")

# Prepare native posix testing on host
set(BOARD "native_posix")
set(ZEPHYR_TOOLCHAIN_VARIANT "host")

set(ZEPHYR_MODULES ${THINGSET_BASE})

find_package(Zephyr REQUIRED HINTS ${zephyr_base})
#include(${zephyr_base}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(thingset_zephyr_tests)

target_sources(app PRIVATE
    ${THINGSET_BASE}/test/main.cpp
    ${THINGSET_BASE}/test/test_assert.c
    ${THINGSET_BASE}/test/test_bin.c
    ${THINGSET_BASE}/test/test_buf.c
    ${THINGSET_BASE}/test/test_common.c
    ${THINGSET_BASE}/test/test_ctx.c
    ${THINGSET_BASE}/test/test_data.c
    ${THINGSET_BASE}/test/test_jsmn.c
    ${THINGSET_BASE}/test/test_msg.c
    ${THINGSET_BASE}/test/test_obj.c
    ${THINGSET_BASE}/test/test_shell.c
    ${THINGSET_BASE}/test/test_shim.cpp
    ${THINGSET_BASE}/test/test_support.c
    ${THINGSET_BASE}/test/test_time.c
    ${THINGSET_BASE}/test/test_txt.c
    # Zephyr implementation test suite
    ${CMAKE_CURRENT_SOURCE_DIR}/test_impl.c
)
