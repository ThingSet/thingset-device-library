# Copyright (c) 2020 Martin Jäger / Libre Solar
# Copyright (c) 2021 Bobby Noelte.
# SPDX-License-Identifier: Apache-2.0

# Make ThingSet library a Zephyr module
# -------------------------------------
#
# Use ZEPHYR_BASE as sentinel to assure we are building with Zephyr
# (Other build environments may try to use this CMakeLists.txt).
if(ZEPHYR_BASE AND DEFINED CONFIG_THINGSET)

# Get base directorie(s).
# Assumption: this file is within the zephyr subdir of thingset (also because of platformio)
get_filename_component(THINGSET_BASE ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# The ThingSet library
# --------------------

zephyr_include_directories(${THINGSET_BASE}/src)

zephyr_library_named("ts")

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
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_rbbq.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/ts_time.c)
# Zephyr implementation
target_sources(ts PRIVATE ${THINGSET_BASE}/zephyr/thingset/ts_impl_buf.c)
if(DEFINED CONFIG_LOG)
    target_sources(ts PRIVATE ${THINGSET_BASE}/zephyr/thingset/ts_impl_log.c)
endif()

target_sources(ts PRIVATE ${THINGSET_BASE}/native/thingset/ts_impl_libc.c)
if(DEFINED CONFIG_MINIMAL_LIBC)
    target_sources(ts PRIVATE ${THINGSET_BASE}/zephyr/thingset/ts_impl_libc_minimal.c)
endif()

if(NOT DEFINED CONFIG_TINYCBOR)
target_sources(ts PRIVATE
    ${THINGSET_BASE}/native/tinycbor/src/cborerrorstrings.c
    ${THINGSET_BASE}/native/tinycbor/src/cborencoder.c
    ${THINGSET_BASE}/native/tinycbor/src/cborencoder_close_container_checked.c
    ${THINGSET_BASE}/native/tinycbor/src/cborencoder_float.c
    ${THINGSET_BASE}/native/tinycbor/src/cborparser.c
    ${THINGSET_BASE}/native/tinycbor/src/cborparser_float.c
    ${THINGSET_BASE}/native/tinycbor/src/cborpretty.c )
endif()

# Ports
add_subdirectory(${THINGSET_BASE}/src/ports/loopback_simple ${THINGSET_BASE}/build/loopback_simple)

# Applications
add_subdirectory(${THINGSET_BASE}/zephyr/apps/shell ${THINGSET_BASE}/build/shell)

endif()
