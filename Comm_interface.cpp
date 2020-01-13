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

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#include "comm_interface.h"

bool Comm_interface::init(){
    BLE &ble_interface = BLE::Instance();
    _bleprocess = new BLEProcess(_event_queue, ble_interface);
    _bleprocess->on_init(NULL);
    _blesda = new BLESDA(ble_interface);
	return true;
}
bool Comm_interface::start(){
    bool status = _bleprocess->start();
    _event_queue.dispatch_forever();
    return status;
}

int Comm_interface::wait_for_message(uint8_t **message_out, uint32_t *message_size_out){
     bool success = false;
	 if(_isconnected == false){
	 	return -1;
     }
    int status_code = 0;
    uint8_t *message = NULL;
    uint32_t message_size = 0;

    *message_out = NULL;
    *message_size_out = 0;
	printf("%s", message);

    *message_out = message;
    *message_size_out = message_size;
    return status_code;
}

bool Comm_interface::send_response(uint8_t *message_out, size_t message_size){
	_blesda->write(message_out, message_size);
}

bool Comm_interface::finish(){}
