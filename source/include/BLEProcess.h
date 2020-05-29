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

#ifndef __BLE_PROCESS_H__
#define __BLE_PROCESS_H__

#include <stdint.h>
#include <stdio.h>
#include "BLE-SDAService.h"
#include "DeviceInformationService.h"
#include "ble/BLE.h"
#include "ble/FunctionPointerWithContext.h"
#include "ble/Gap.h"
#include "ble/GapAdvertisingData.h"
#include "ble/GapAdvertisingParams.h"
#include "events/EventQueue.h"
#include "platform/Callback.h"
#include "platform/NonCopyable.h"
#include "version.h"

#define TRACE_GROUP "blep"
/**
 * Handle initialization adn shutdown of the BLE Instance.
 *
 * Setup advertising payload and manage advertising state.
 * Delegate to GattClientProcess once the connection is established.
 */

class BLEProcess : private mbed::NonCopyable<BLEProcess> {
   public:
	/**
	 * Construct a BLEProcess from an event queue and a ble interface.
	 *
	 * Call start() to initiate ble processing.
	 */
	BLEProcess(events::EventQueue &event_queue, BLE &ble_interface,
			   char *endpoint)
		: _event_queue(event_queue),
		  _ble_interface(ble_interface),
		  _endpoint(endpoint),
		  _post_init_cb() {}
	~BLEProcess() { stop(); }

	/**
	  * Subscription to the ble interface initialization event.
	  *
	  * @param[in] cb The callback object that will be called when the ble
	  * interface is initialized.
	  */
	void on_init(mbed::Callback<void(BLE &, events::EventQueue &)> cb);

	/**
	 * Initialize the ble interface, configure it and start advertising.
	 */
	bool start();
	/**
	 * Close existing connections and stop the process.
	 */
	void stop();
	/*returns the connection status of ble*/

   private:
	void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *event);
	void when_init_complete(BLE::InitializationCompleteCallbackContext *event);
	void print_ble(const char *str);
	void getc();
	void when_connection(
		const Gap::ConnectionCallbackParams_t *connection_event);
	void when_disconnection(const Gap::DisconnectionCallbackParams_t *event);
	bool start_advertising(void);
	bool set_advertising_parameters();
	bool set_advertising_data();
	int getIndex();
	char *getBuff();
	void wait();
	events::EventQueue &_event_queue;
	BLE &_ble_interface;
	char *_endpoint;
	BLESDA *blesda = NULL;
	mbed::Callback<void(BLE &, events::EventQueue &)> _post_init_cb;
};
#endif /* GATT_SERVER_EXAMPLE_BLE_PROCESS_H_ */
