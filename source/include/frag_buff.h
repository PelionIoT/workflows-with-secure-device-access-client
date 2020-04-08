

/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
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
#ifndef __FRAG_BUFF__
#define __FRAG_BUFF__

#include "events/EventQueue.h"
#include "ble/UUID.h"
#include "ble/BLE.h"
#include "ble/pal/Deprecated.h"
#include "SDAOperation.h"
#include "mbed_trace.h"
#include "mbed.h"
#include "./../../ble-config-uuid.h"

#define TRACE_GROUP_BLE             "BLESDA"
#define UPPER_BYTE_REQ_INDEX    3
#define LOWER_BYTE_REQ_INDEX    4
#define START_DATA_BYTE         8
#define SDA_DATA                1
#define SDA_ACK                 3
#define SDA_REQ                 2

typedef struct _frag_buff {
	 /* Frame sequence number(1byte) */
	uint8_t seq_num;
    /* control frame field 2byte) */
	uint8_t type:2;
    uint8_t more_frag:1;
	uint8_t frag_num:3;
    /* Frag length (1byte) */
    uint8_t frag_length;
    /* Total length (2byte) */
    uint16_t length;
    uint8_t resv1:2;
	uint8_t resv2;
    uint8_t resv3;
    uint16_t resv4;
	/* Payload */
    uint8_t* payload;
}__attribute__((packed)) frag_buff;

frag_buff buff_init();
bool get_header(uint8_t* arr,frag_buff* buff);
bool get_payload(uint8_t*arr, frag_buff* buff);
bool populate_header(frag_buff* buff, const uint8_t* header_data);
bool populate_data(frag_buff* buff, const uint8_t* payload_data);
bool is_last_fragment(frag_buff* buff);

#endif












