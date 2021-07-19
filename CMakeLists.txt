# SPDX-License-Identifier: Apache-2.0

zephyr_include_directories(src)

target_sources(app PRIVATE src/thingset.c)
target_sources(app PRIVATE src/thingset_bin.c)
target_sources(app PRIVATE src/thingset_txt.c)
target_sources(app PRIVATE src/cbor.c)
target_sources(app PRIVATE src/jsmn.c)
