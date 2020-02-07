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

#include "blesda.h"
#ifndef _PROTOCOL_TRANSLATOR_H_
#define _PROTOCOL_TRANSLATOR_H_

#define FTCD_MSG_HEADER_TOKEN_SDA { 0x6d, 0x62, 0x65, 0x64, 0x64, 0x62, 0x61, 0x70 }
#define FTCD_MSG_HEADER_TOKEN_SIZE_BYTES 8

#define HEADER_START            0
#define HEADER_SIZE             11
#define START_DATA              12
enum PTErr{
    PT_ERR_OK,
    PT_ERR_HEADER_MISMATCH,
    PT_ERR_MSG,
    PT_ERR_MSG_SIG,
    PT_COMM_FAILED_TO_CALCULATE_MESSAGE_SIGNATURE,
    PT_COMM_INCONSISTENT_MESSAGE_SIGNATURE,
    PT_ERR_PROCESS_REQ,
};
class ProtocolTranslator{
    public:
        ProtocolTranslator(uint8_t* request):
        _message_header(FTCD_MSG_HEADER_TOKEN_SDA),_index(0),_buffer(request),_message_size(0) {};
        PTErr init(uint8_t* response,uint8_t response_max_size, uint16_t* response_size);
        PTErr is_token_detected();
        uint32_t read_message_size();
        PTErr read_message(uint8_t* message, uint32_t message_size);
        PTErr read_message_signature(uint8_t* message, size_t message_size);

    private:
        int _index;
        uint32_t _message_size;
        size_t _read(uint8_t* buffer, size_t message_size);
        size_t _write(uint8_t* buffer, size_t message_size);
        uint8_t* _buffer;
        uint8_t _message_header[FTCD_MSG_HEADER_TOKEN_SIZE_BYTES];
        bool _use_signature;
};

#endif
