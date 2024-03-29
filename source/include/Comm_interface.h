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

#ifndef __COMM_INTERFACE_H__
#define __COMM_INTERFACE_H__

#include "BLE-SDAService.h"
#include "BLEProcess.h"

class Comm_interface {
   public:
	Comm_interface(){};
	bool init(char* endpoint);
	bool start();

   private:
	events::EventQueue _event_queue;
	BLEProcess* _bleprocess = NULL;
};
#endif
