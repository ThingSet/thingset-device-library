# Copyright (c) 2021 Bobby Noelte.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

project(native_thingset_test)

# Get base directory.
# Assumption: this file is within the zephyr/tests subdir of the thingset library.
get_filename_component(THINGSET_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
message(STATUS "THINGSET_BASE: ${THINGSET_BASE}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${THINGSET_BASE}/src
    ${THINGSET_BASE}/native/unity/src)

add_executable(test
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
    # Native implementation test suite
    ${CMAKE_CURRENT_SOURCE_DIR}/test_impl.c
)

add_library(ts STATIC "")
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_app.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_cobs.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_cmd.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_core.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_export.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_msg.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_node.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_obj.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_ctx_process.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_jsmn.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_coder.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_export.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_log.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_proto.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_msg_value.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_obj.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_obj_log.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_port.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_time.c)
# Native implementation
# - buf, bufq, time, libc
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_buf.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_bufq.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_libc.c)
# - TinyCBOR support lib
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborencoder.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborencoder_close_container_checked.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborencoder_float.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborerrorstrings.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborparser.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborparser_float.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborpretty.c)
# - Unit test by Unity
target_sources(ts PRIVATE ${THINGSET_BASE}/native/unity/src/unity.c)

# ports
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ports/loopback_simple/loopback_simple.c)

# applications
target_sources(ts PRIVATE ${THINGSET_BASE}/apps/shell/ts_shell.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/apps/shell/ts_shell_abuf.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/apps/shell/ts_shell_g.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_shell.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_shell_linenoise.c)

target_link_libraries(test ts)

# for math.h functions
target_link_libraries(test m)

# for thingset device library configuration by header file
add_definitions(-g -DTHINGSET_CONFIG_HEADER="config.h" -fprofile-arcs -ftest-coverage)

target_link_libraries(test gcov)
