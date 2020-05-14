// ----------------------------------------------------------------------------
// Copyright 2017-2019 ARM Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include "events/EventQueue.h"
#include "include/BLEProcess.h"
#include "include/Comm_interface.h"

bool Comm_interface::init(char* endpoint)
{
	BLE &ble_interface = BLE::Instance();
	_bleprocess = new BLEProcess(_event_queue, ble_interface, endpoint);
	if(!_bleprocess)
	{
		return false;
	}
	_bleprocess->on_init(NULL);
	return true;
}
bool Comm_interface::start()
{
	bool status = _bleprocess->start();
	if(status)
	{
		event_queue.dispatch_forever();
	}
	return status;
}
