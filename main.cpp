/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
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

#include <stdio.h>
#include "pretty_printer.h"
#include <stdint.h>
#include <stdio.h>
#include "events/EventQueue.h"
#include "platform/Callback.h"
#include "platform/NonCopyable.h"

#include "BLEProcess.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GapAdvertisingParams.h"
#include "ble/GapAdvertisingData.h"
#include "ble/services/UARTService.h"

#include "string" // for string class 


using mbed::callback;


int main(void) {
	events::EventQueue event_queue;
	BLE &ble_interface = BLE::Instance();
	BLEProcess ble_process(event_queue, ble_interface);
	ble_process.on_init(NULL);
	ble_process.start();
	event_queue.dispatch_forever();
}