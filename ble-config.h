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

//Define BLE packet Length that will transmit in each transition, should be '8' Less than MTU.

#define BLE_PACKET_SIZE             0

/***
For Device Information Service in BLE
 * Device Local Name to advertise in BLE
 * Manufacturer Name
 * Model Number
 * Hardware Revision
 * Firmware Revision
 * Software Revision
*/
#define Device_Local_Name
#define MANUFACTURER_NAME
#define MODEL_NUM
#define HARDWARE_REVISION
#define FIRMWARE_REVISION
#define SOFTWARE_REVISION

const uint8_t  ServiceBaseUUID[LONG_UUID_LENGTH]    = {0};
const uint16_t ServiceShortUUID                     = 0x0000;
const uint16_t CharacteristicShortUUID              = 0x0000;
const uint8_t  ServiceUUID[LONG_UUID_LENGTH]        = {0};
const uint8_t  CharacteristicUUID[LONG_UUID_LENGTH] = {0};

#if BLE_PACKET_SIZE == 0
    #error "Fill the configurations with the details like UUID, Name, BLE_PACKET_SIZE etc!"
#endif
#endif