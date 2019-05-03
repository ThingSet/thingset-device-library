/* ThingSet protocol library
 * Copyright (c) 2017-2018 Martin Jäger (www.libre.solar)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __TS_CONFIG_H_
#define __TS_CONFIG_H_

/* Maximum number of expected JSON tokens (i.e. arrays, map keys, values,
 * primitives, etc.)
 *
 * Thingset throws an error if maximum number of tokens is reached in a
 * request or response.
 */
#ifndef TS_NUM_JSON_TOKENS
#define TS_NUM_JSON_TOKENS 50
#endif

/* If verbose status messages are switched on, a response in text-based mode
 * contains not only the status code, but also a message.
 */
#ifndef TS_VERBOSE_STATUS_MESSAGES
#define TS_VERBOSE_STATUS_MESSAGES 1
#endif

/* Switch on support for 64 bit variable types (uint64_t, int64_t, double)
 *
 * This should be commented out for most 8-bit microcontrollers to increase
 * performance
 */
#ifndef TS_64BIT_TYPES_SUPPORT
#define TS_64BIT_TYPES_SUPPORT 0        // default: no support
#endif

#endif /* __TS_CONFIG_H_ */
