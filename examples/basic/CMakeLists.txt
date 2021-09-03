# Copyright (c) 2021 Martin Jäger / Libre Solar
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)

project(basic_thingset_example)

get_filename_component(THINGSET_BASE ${CMAKE_CURRENT_SOURCE_DIR}/../../.. DIRECTORY)

include_directories(${THINGSET_BASE}/src)

add_executable(sample
    main.c
)

add_library(ts STATIC "")
add_subdirectory(${THINGSET_BASE}/src build/ts)
target_link_libraries(sample ts)

# for math.h functions
target_link_libraries(sample m)
