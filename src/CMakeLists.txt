# Copyright (c) 2021 Martin Jäger / Libre Solar
# Copyright (c) 2021 Bobby Noelte.
#
# SPDX-License-Identifier: Apache-2.0

target_sources(ts PRIVATE ${THINGSET_BASE}/src/thingset.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/thingset_bin.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/thingset_txt.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/cbor.c)
target_sources(ts PRIVATE ${THINGSET_BASE}/src/jsmn.c)
