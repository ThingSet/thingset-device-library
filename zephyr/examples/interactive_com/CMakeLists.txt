# Copyright (c) 2021 Bobby Noelte.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.13.1)

# Get base directory.
# Assumption: this file is within the zephyr/samples/basic subdir of the thingset library.
get_filename_component(THINGSET_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
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
        break()
    endif()
endforeach()
message(STATUS "ZEPHYR_BASE: ${zephyr_base}")

# Prepare native posix example on host
set(BOARD "native_posix")
set(ZEPHYR_TOOLCHAIN_VARIANT "host")

set(ZEPHYR_MODULES ${THINGSET_BASE})

find_package(Zephyr REQUIRED HINTS ${zephyr_base})
project(thingset_example_interactive_com)

target_sources(app PRIVATE
    src/main.c
    # test data & support
    ${THINGSET_BASE}/test/test_support.c
    ${THINGSET_BASE}/test/test_data.c
)

