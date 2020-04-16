/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 */

#ifndef TESTS_H_
#define TESTS_H_

#define TS_REQ_BUFFER_LEN 500
#define TS_RESP_BUFFER_LEN 500

void tests_common();
void tests_json();
void tests_cbor();

#endif /* TESTS_H_ */
