# Copyright (c) 2021 Martin Jäger / Libre Solar
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.10)

project(basic_thingset_example)

get_filename_component(THINGSET_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. DIRECTORY)

include_directories(${THINGSET_BASE}/src)
include_directories(${CMAKE_CURRENT_LIST_DIR}/linenoise)

set(CMAKE_CXX_FLAGS -pthread)

add_executable(sample
    main.cpp
    linenoise/linenoise.c
)

add_library(ts STATIC "")
add_subdirectory(${THINGSET_BASE}/src build/ts)
target_link_libraries(sample ts)
