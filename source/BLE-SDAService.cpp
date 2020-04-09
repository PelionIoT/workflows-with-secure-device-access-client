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
#include "include/BLE-SDAService.h"
using mbed::callback;

#define response_size 1000
uint16_t idx = 0;
uint8_t g_seqnum = 0;
uint16_t packet = 0;
uint16_t BLESDA::getCharacteristicHandle() {
    return sdaCharacteristic.getValueAttribute().getHandle();
}

size_t BLESDA::write(uint8_t* buff, uint8_t length) {
    if (ble.gap().getState().connected) {
            ble.gattServer().write(getCharacteristicHandle(), static_cast<const uint8_t *>(buff), length);
    }
    else{
        ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
        mbed_tracef(TRACE_ACTIVE_LEVEL_ERROR, TRACE_GROUP_BLE, "ble not connected, advertising started");
        return 0;
    }
	return length;
}

sda_protocol_error_t BLESDA::BLETX(Frag_buff* header, uint8_t len){
    if(ble.gap().getState().connected) {
        uint8_t transmit_data_len = len+START_DATA_BYTE+1;
        uint8_t* msg = (uint8_t*)malloc(transmit_data_len*sizeof(uint8_t));
        memcpy(msg, header, START_DATA_BYTE);
        memcpy(&msg[START_DATA_BYTE], header->payload, len);
        write(msg, transmit_data_len);
        free(msg);
        return PT_ERR_OK;
    }
    else {
        ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
        return PT_ERR_LOST_CONN;
    }
}


//function that process buffer according to BLEP protocol
sda_protocol_error_t BLESDA::sda_fragment_datagram(uint8_t* sda_payload, uint16_t payloadsize, uint8_t type)
{
    if(ble.gap().getState().connected) {
        uint8_t fragmentStartOffset=0;
        static uint8_t frag_num=0;
        uint16_t totalpayloadsize=payloadsize;
        //Calculate number of fragment in order to be transmitted
        uint8_t num_frag = ((payloadsize%BLE_PACKET_SIZE) == 0)?(payloadsize/BLE_PACKET_SIZE):(payloadsize/BLE_PACKET_SIZE)+1;
        while(num_frag)
        {
            uint8_t more_frag = (num_frag==1)?0:1;
            uint16_t frag_length=(payloadsize < BLE_PACKET_SIZE)? payloadsize : BLE_PACKET_SIZE;

            Frag_buff frag_sda = get_buff(g_seqnum, type, more_frag, ++frag_num, totalpayloadsize,frag_length);
            bool success = populate_fragment_data(&frag_sda,&sda_payload[fragmentStartOffset]);
            if(!success)
                {
                    mbed_tracef(TRACE_LEVEL_ERROR,TRACE_GROUP_BLE,"can not create payload for BLE");
                    return PT_ERR_SEND_BLE;
                }
            if(payloadsize < BLE_PACKET_SIZE)
            {
                fragmentStartOffset = 0;
            }
            else
            {
                fragmentStartOffset+=frag_sda.frag_length;
            }
            //Transmit it to BLE
            sda_protocol_error_t status = BLETX(&frag_sda,frag_sda.frag_length);
            if(status != PT_ERR_OK){
                free(frag_sda.payload);
                return status;
            }
            num_frag--;
            payloadsize -= frag_sda.frag_length;
            //free allocated memory
            free(frag_sda.payload);
        }
        g_seqnum = g_seqnum+1;
        return PT_ERR_OK;
    }
    else{
        ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
        mbed_tracef(TRACE_LEVEL_ERROR, TRACE_GROUP_BLE, "connection lost..now advertising!");
        return PT_ERR_LOST_CONN;
    }
 }


sda_protocol_error_t BLESDA::ProcessBuffer(Frag_buff* frag_sda){
    if(ble.gap().getState().connected) {
        uint8_t response[response_size]={0};
        uint16_t sda_response_size=0;
        if(msg_to_sda==NULL){
            msg_to_sda = (uint8_t*)malloc(frag_sda->length*sizeof(uint8_t));
        }
        if(msg_to_sda == NULL){
            mbed_tracef(TRACE_LEVEL_ERROR,TRACE_GROUP_BLE,"Could not allocate memory for message to sda");
            return PT_ERR_MSG;
        }
        memcpy(&msg_to_sda[idx],frag_sda->payload, frag_sda->frag_length);
        if(!is_last_fragment(frag_sda)){
            idx +=frag_sda->frag_length;
            return PT_ERR_OK;
        }
        else{
            idx = 0;
            mbed_tracef(TRACE_LEVEL_INFO,TRACE_GROUP_BLE,"Sending buffer to SDA\r\n");
            SDAOperation sda_operation(msg_to_sda);
            sda_protocol_error_t status = sda_operation.init(response,response_size,&sda_response_size);
            free(msg_to_sda);
            msg_to_sda = NULL;
            if(status !=PT_ERR_OK){
                return status;
            }
            return sda_fragment_datagram(response, sda_response_size, SDA_DATA);
        }
    }
    else{
        ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
        if(msg_to_sda){
            free(msg_to_sda);
        }
        return PT_ERR_LOST_CONN;
    }
}

void BLESDA::onDataWritten(const GattWriteCallbackParams *params) {
    packet+=params->len;
    mbed_tracef(TRACE_LEVEL_INFO,TRACE_GROUP_BLE,"Data Received %d",packet);
    if (params->handle == getCharacteristicHandle()) {
    Frag_buff buffer = {0};
    bool success = populate_header(&buffer, params->data);
    if(!success){
        return;
    }
    success = populate_data(&buffer, params->data);
    if(!success){
        return;
    }
    sda_protocol_error_t status = ProcessBuffer(&buffer);
    free(buffer.payload);
    if(status != PT_ERR_OK){
        mbed_tracef(TRACE_LEVEL_ERROR, TRACE_GROUP_BLE, "Got err: (%d)",status);
        return;
    	}
    }
    else{
        mbed_tracef(TRACE_LEVEL_ERROR, TRACE_GROUP_BLE, "can not get characteristic handle");
        return;
    }
}