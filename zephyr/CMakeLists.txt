# Copyright (c) 2020 Martin Jäger / Libre Solar
# Copyright (c) 2021 Bobby Noelte.
# SPDX-License-Identifier: Apache-2.0

# Make library a Zephyr module
#
# Use ZEPHYR_BASE as sentinel to assure we are building with Zephyr
# (Other build environments may try to use this CMakeLists.txt).
if(ZEPHYR_BASE AND DEFINED CONFIG_THINGSET)

# Get base directorie(s).
# Assumption: this file is within the zephyr subdir of thingset (also because of platformio)
get_filename_component(THINGSET_BASE ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

# The ThingSet library
# --------------------

zephyr_library_named("ts")

zephyr_include_directories(${THINGSET_BASE}/src)

add_subdirectory(${THINGSET_BASE}/src build/thingset)

if(DEFINED CONFIG_MINIMAL_LIBC)
    target_sources(ts PRIVATE ${THINGSET_BASE}/zephyr/libc/minimal/extensions.c)
endif()

endif()
