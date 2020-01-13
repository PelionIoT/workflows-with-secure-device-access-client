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
#ifndef _COMM_INTERFACE_H_
#define _COMM_INTERFACE_H_
#endif
#include "events/EventQueue.h"
#include "BLEProcess.h"
#include "blesda.h"
//#include "ftcd_comm_base.h"


class Comm_interface{
    public:
        Comm_interface(){
        };
        bool init();
        bool start();
        int wait_for_message(uint8_t **message_out, uint32_t *message_size_out);
        bool send_response(uint8_t* response, size_t message_size);
        bool finish();
    private:
        bool isconnected();
        events::EventQueue _event_queue;
        BLEProcess* _bleprocess;
        BLESDA* _blesda;
        bool _isconnected = false;
};