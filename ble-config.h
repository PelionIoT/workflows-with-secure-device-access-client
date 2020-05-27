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

#ifndef __BLE_CONFIG_UUID__
#define __BLE_CONFIG_UUID__

#include <stdint.h>

#define LONG_UUID_LENGTH            16

/***
For Device Information Service in BLE
 * Device Local Name to advertise in BLE
 * Manufacturer Name
 * Model Number
 * Hardware Revision
 * Firmware Revision
 * Software Revision
 * Define BLE packet Length that will transmit in each transition, should be '8' Less than MTU.
 */
#define BLE_PACKET_SIZE             200
#define Device_Local_Name           "DISCO"
#define MANUFACTURER_NAME           "ARM"
#define MODEL_NUM                   "0.1.1"
#define HARDWARE_REVISION           "0.1.1"
#define FIRMWARE_REVISION           "0.1.1"
#define SOFTWARE_REVISION           "0.1.1"
#define MOUNT_POINT                 "fs"

const uint8_t  ServiceBaseUUID[LONG_UUID_LENGTH] = {
    0x6E, 0x40, 0x00, 0x00, 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};

const uint16_t ServiceShortUUID                 = 0x0001;
const uint16_t CharacteristicShortUUID          = 0x0002;

const uint8_t  ServiceUUID[LONG_UUID_LENGTH] = {
    0x6E, 0x40, (uint8_t)(ServiceShortUUID >> 8), (uint8_t)(ServiceShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};

const uint8_t  CharacteristicUUID[LONG_UUID_LENGTH] = {
    0x6E, 0x40, (uint8_t)(CharacteristicShortUUID >> 8), (uint8_t)(CharacteristicShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};

#if BLE_PACKET_SIZE == 0
    #error "Fill the configurations with the details like UUID, Name, BLE_PACKET_SIZE etc!"
#endif
#endif