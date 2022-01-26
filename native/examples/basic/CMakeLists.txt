# Copyright (c) 2021 Martin Jäger / Libre Solar
# Copyright (c) 2022 Bobby Noelte
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)

project(thingset_example_basic)

get_filename_component(THINGSET_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../../.. DIRECTORY)
message(STATUS "THINGSET_BASE: ${THINGSET_BASE}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${THINGSET_BASE}/src)

add_executable(example
     ${THINGSET_BASE}/examples/basic/main.c
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
# Native implementation
# - buf, bufq, time
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_buf.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_bufq.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_time.c)
# - TinyCBOR support lib
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborencoder.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborencoder_close_container_checked.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborencoder_float.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborerrorstrings.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborparser.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborparser_float.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/native/tinycbor/src/cborpretty.c)

target_link_libraries(example ts)

# for math.h functions
target_link_libraries(example m)

# for thingset device library configuration by header file
add_definitions(-DTHINGSET_CONFIG_HEADER="config.h")
