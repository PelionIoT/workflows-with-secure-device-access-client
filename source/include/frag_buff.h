/*
 * ----------------------------------------------------------------------------
 * Copyright 2020 ARM Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
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
 * ----------------------------------------------------------------------------
 */

#ifndef __FRAG_BUFF_H__
#define __FRAG_BUFF_H__

#include <stdint.h>
#include <stdlib.h>
#include <cstddef>
#include <cstring>

#define START_DATA_BYTE 8

typedef struct _Frag_buff {
	uint8_t seq_num;		// global seq number that contains the packet number
	uint8_t type : 2;		// type- control frame or data frame
	uint8_t more_frag : 1;  // More fragment bit- it tells if there are more
							// packets or not. if set- then there are more.
	uint8_t frag_num : 3;   // Fragment Number
	uint8_t resv1 : 2;		// reserve
	uint8_t frag_length;	// Legth of fragment
	uint16_t length;		// total length
	uint8_t resv2;
	uint8_t resv3;
	uint16_t resv4;
	/* Payload */
	uint8_t* payload;  // payload, contains the data.
} __attribute__((packed)) Frag_buff;
// returns the buffer without the payload.
Frag_buff get_buff(uint8_t seq_num, uint8_t type, uint8_t more_frag,
				   uint8_t frag_num, uint8_t frag_length, uint16_t length);
// returns the header
bool get_header(uint8_t* arr, Frag_buff* buff);
// returns data payload
bool get_payload(uint8_t* arr, Frag_buff* buff);

bool populate_header(Frag_buff* buff, const uint8_t* header_data);
bool populate_data(Frag_buff* buff, const uint8_t* payload_data);
bool populate_fragment_data(Frag_buff* buff, const uint8_t* data);
bool is_last_fragment(Frag_buff* buff);
#endif
