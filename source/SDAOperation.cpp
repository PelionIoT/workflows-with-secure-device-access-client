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

#include "include/SDAOperation.h"
#include "pv_endian.h"
#include "cs_hash.h"
#include "kcm_defs.h"
#include "SDAHelper.h"

#define TRACE_GROUP_PT "PT"

size_t SDAOperation::_read(uint8_t* message, size_t message_size){
    if(memcpy(message, &_buffer[START_DATA],message_size)==NULL){
        mbed_tracef(TRACE_ACTIVE_LEVEL_ERROR, TRACE_GROUP_PT,"Can not read message");
        return MBED_ERROR_ALLOC_FAILED;
    }
    _index +=message_size;
    return message_size;
}

sda_protocol_error_t SDAOperation::is_token_detected(){
    for(int i = 0; i< FTCD_MSG_HEADER_TOKEN_SIZE_BYTES; i++) {
        if(_buffer[i]!=_message_header[i]){
            return PT_ERR_HEADER_MISMATCH;
        }
    }        return PT_ERR_OK;
}

uint32_t SDAOperation::read_message_size(){
    uint32_t message_size = (_buffer[8]<<24)+
                            (_buffer[9]<<16)+
                            (_buffer[10]<<8)+
                            _buffer[11];
    return message_size;
}
// read message which goes to SDA.
sda_protocol_error_t SDAOperation::read_message(uint8_t* message, uint32_t message_size){
    if(memcpy(message,&_buffer[START_DATA], message_size)==NULL){
        return PT_ERR_MSG;
    }
    return PT_ERR_OK;
}
sda_protocol_error_t SDAOperation::read_message_signature(uint8_t* sig, size_t req_size){
    if(memcpy(sig, &_buffer[HEADER_BYTE+_message_size+1], req_size) == NULL){
        return PT_ERR_MSG_SIG;
    }
    return PT_ERR_OK;
}

sda_protocol_error_t SDAOperation::init(uint8_t* response, uint8_t response_max_size, uint16_t* response_size){
   // printf("Response maxsize %ld\r\n",response_max_size);
    size_t response_actual_size = 0;
    // for(int i = 0 ; i < 8; i++){
    //     printf("%d ", _buffer[i]);
    // }
    sda_protocol_error_t status = is_token_detected();
    if(status != PT_ERR_OK){
        mbed_tracef(TRACE_LEVEL_INFO,TRACE_GROUP_PT,"Token not detected");
        return status;
    }
    _message_size = read_message_size();
    uint8_t* msg = (uint8_t*)malloc(_message_size);
    if(msg==NULL){
        mbed_tracef(TRACE_LEVEL_ERROR, TRACE_GROUP_PT,"Can not init message to process SDA");
        return PT_ERR_MSG;
    }
    if(read_message(msg,_message_size)!=PT_ERR_OK) {
        mbed_tracef(TRACE_LEVEL_ERROR,TRACE_GROUP_PT, "not able to get message %ld",_message_size);
        free(msg);
        return PT_ERR_MSG;
    }
    uint8_t sig_from_message[KCM_SHA256_SIZE];
    status = read_message_signature(sig_from_message, sizeof(sig_from_message));
    if(status != PT_ERR_OK) {
        //printf("err reading msg sig");
        mbed_tracef(TRACE_LEVEL_ERROR, TRACE_GROUP_PT, "err reading message");
        free(msg);
        return status;
    }
    bool success = process_request_fetch_response(msg, _message_size, response, response_max_size, &response_actual_size);
        if (!success) {
            mbed_tracef(TRACE_LEVEL_ERROR,TRACE_GROUP_PT,"Failed processing request message");
            free(msg);
            return  PT_ERR_PROCESS_REQ;
        }
        // for(int i = 0; i< response_actual_size; i++)
        //     mbed_tracef(TRACE_LEVEL_ERROR, "PT", " %d ", response[i]);
        *response_size = response_actual_size;

        uint8_t self_calculated_sig[KCM_SHA256_SIZE];
        kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
        kcm_status = cs_hash(CS_SHA256, msg, _message_size, self_calculated_sig, sizeof(self_calculated_sig));
        if (kcm_status != KCM_STATUS_SUCCESS) {
            //printf("failed calculating msg sig");
            mbed_tracef(TRACE_LEVEL_CMD, TRACE_GROUP_PT, "Failed calculating message signature");
            status = PT_ERR_FAILED_TO_CALCULATE_MESSAGE_SIG;
            free(msg);
            return status;
        }

    //compare signatures
        if (memcmp(self_calculated_sig, sig_from_message, KCM_SHA256_SIZE) != 0) {
            mbed_tracef(TRACE_LEVEL_CMD, TRACE_GROUP_PT, "Inconsistent message signature");
            status = PT_ERR_INCONSISTENT_MESSAGE_SIG;
            free(msg);
            return status;
        }
    free(msg);
    return PT_ERR_OK;
}