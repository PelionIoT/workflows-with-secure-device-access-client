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

#include "include/protocoltranslator.h"
#include "mbed-trace/mbed_trace.h"
#include "pv_endian.h"
#include "cs_hash.h"
#include "kcm_defs.h"
#include "sdahelper.h"

#define TRACE_GROUP "PT"

size_t ProtocolTranslator::_read(uint8_t* message, size_t message_size){
    if(memcpy(message, &_buffer[START_DATA],message_size)==NULL){
        mbed_tracef(TRACE_ACTIVE_LEVEL_ERROR, TRACE_GROUP,"Can not read message");
        return MBED_ERROR_ALLOC_FAILED;
    }
    _index +=message_size;
    return message_size;
}

size_t ProtocolTranslator::_write(uint8_t* message, size_t message_size){
   // return _blesdahandler->write(message, message_size);
}


PTErr ProtocolTranslator::is_token_detected(){
    for(int i = 0; i< FTCD_MSG_HEADER_TOKEN_SIZE_BYTES; i++) {
        if(_buffer[i]!=_message_header[i]){
            return PT_ERR_HEADER_MISMATCH;
        }
    }
     _index +=FTCD_MSG_HEADER_TOKEN_SIZE_BYTES;
        return PT_ERR_OK;
}

uint32_t ProtocolTranslator::read_message_size(){
    uint32_t message_size = (_buffer[_index]<<24)+
                            (_buffer[++_index]<<16)+
                            (_buffer[++_index]<<8)+
                            _buffer[++_index];
    return message_size;
}
// read message which goes to SDA.
PTErr ProtocolTranslator::read_message(uint8_t* message, uint32_t message_size){
    if(memcpy(message,&_buffer[START_DATA], message_size)==NULL){
        return PT_ERR_MSG;
    }
    _index+=message_size;
    return PT_ERR_OK;
}
PTErr ProtocolTranslator::read_message_signature(uint8_t* sig, size_t req_size){
    if(memcpy(sig, &_buffer[HEADER_BYTE+_message_size+1], req_size) == NULL){
        return PT_ERR_MSG_SIG;
    }
    return PT_ERR_OK;
}

PTErr ProtocolTranslator::init(uint8_t* response, uint8_t response_max_size, uint16_t* response_size){
   // printf("Response maxsize %ld\r\n",response_max_size);
    size_t response_actual_size;
    //printf("In PT\r\n");
    PTErr status = is_token_detected();
    if(status != PT_ERR_OK){
        mbed_tracef(TRACE_ACTIVE_LEVEL_INFO,TRACE_GROUP,"Token not detected");
        return status;
    }
    _message_size = read_message_size();
    mbed_tracef(TRACE_ACTIVE_LEVEL_INFO,TRACE_GROUP, "message size: %d",_message_size);
    uint8_t* msg = (uint8_t*)malloc(_message_size);
    if(msg==NULL){
        mbed_tracef(TRACE_LEVEL_ERROR, TRACE_GROUP,"Can not init message to process ot SDA");
        return PT_ERR_MSG;
    }
    if(read_message(msg,_message_size)!=PT_ERR_OK) {
        mbed_tracef(TRACE_LEVEL_ERROR,TRACE_GROUP, "not able to get message %d",_message_size);
    }
    uint8_t sig_from_message[KCM_SHA256_SIZE];
    status = read_message_signature(sig_from_message, sizeof(sig_from_message));
    if(status != PT_ERR_OK) {
        mbed_tracef(TRACE_LEVEL_ERROR, TRACE_GROUP, "err reading message");
        return status;
    }
    bool success = process_request_fetch_response(msg, _message_size, response, response_max_size, &response_actual_size);
        if (!success) {
            tr_error("Failed processing request message");
            free(msg);
            free(_buffer);
            return  PT_ERR_PROCESS_REQ;
        }
        // for(int i = 0; i< response_actual_size; i++)
        //     mbed_tracef(TRACE_LEVEL_ERROR, "PT", " %d ", response[i]);
        *response_size = response_actual_size;

        uint8_t self_calculated_sig[KCM_SHA256_SIZE];
        kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
        PTErr status_code;
        kcm_status = cs_hash(CS_SHA256, msg, _message_size, self_calculated_sig, sizeof(self_calculated_sig));
        if (kcm_status != KCM_STATUS_SUCCESS) {
            mbed_tracef(TRACE_LEVEL_CMD, TRACE_GROUP, "Failed calculating message signature");
            status_code = PT_ERR_FAILED_TO_CALCULATE_MESSAGE_SIG;
            free(msg);
            return status_code;
        }

    //compare signatures
        if (memcmp(self_calculated_sig, sig_from_message, KCM_SHA256_SIZE) != 0) {
            mbed_tracef(TRACE_LEVEL_CMD, TRACE_GROUP, "Inconsistent message signature");
            status_code = PT_ERR_INCONSISTENT_MESSAGE_SIG;
            free(msg);
            return status_code;
        }
        if(*msg){
            free(msg);
        }
    status_code = PT_ERR_OK;
    return status_code;
}