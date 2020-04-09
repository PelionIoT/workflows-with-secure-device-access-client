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

sda_protocol_error_t BLESDA::BLETX(_sda_over_ble_header* header, uint8_t len){
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
        printf("Here");
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
        uint8_t num_frag = ((payloadsize%BLE_PACKET_SIZE)==0)?(payloadsize/BLE_PACKET_SIZE):(payloadsize/BLE_PACKET_SIZE)+1;
        while(num_frag)
        {
            sda_over_ble_header  frag_sda ={0};
            frag_sda.seq_num = g_seqnum;
            frag_sda.type = type;
            frag_sda.more_frag = (num_frag==1)?0:1;
            frag_sda.frag_num=(++frag_num);
            frag_sda.length=totalpayloadsize;
            frag_sda.frag_length=(payloadsize < BLE_PACKET_SIZE)? payloadsize : BLE_PACKET_SIZE;
            frag_sda.payload = (uint8_t*) malloc(frag_sda.frag_length);
            if(frag_sda.payload!=NULL)
                {
                    memcpy(&frag_sda.payload[0],&sda_payload[fragmentStartOffset],frag_sda.frag_length);
                }
                else
                {
                    mbed_tracef(TRACE_LEVEL_ERROR,TRACE_GROUP_BLE,"can not create payload for BLE");
                    return PT_ERR_SEND_BLE;
                }
            if(payloadsize<BLE_PACKET_SIZE)
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
        printf("Here sda frag");
        ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
        mbed_tracef(TRACE_LEVEL_ERROR, TRACE_GROUP_BLE, "connection lost");
        return PT_ERR_LOST_CONN;
    }
 }


sda_protocol_error_t BLESDA::ProcessBuffer(sda_over_ble_header* frag_sda){
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
        if(frag_sda->more_frag == 1){
        idx +=frag_sda->frag_length;
        return PT_ERR_OK;
        }
        else{
            idx = 0;
            mbed_tracef(TRACE_LEVEL_INFO,TRACE_GROUP_BLE,"Sending buffer to SDA\r\n");
            SDAOperation sda_operation(msg_to_sda);
            sda_protocol_error_t status = sda_operation.init(response,response_size,&sda_response_size);
            if(status !=PT_ERR_OK){
                free(msg_to_sda);
                msg_to_sda = NULL;
                return status;
            }
            free(msg_to_sda);
            msg_to_sda = NULL;
            return sda_fragment_datagram(response, sda_response_size, SDA_DATA);
        }
    }
    else{
        printf("Process BUff");
        ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
        if(msg_to_sda){
            free(msg_to_sda);
        }
        return PT_ERR_LOST_CONN;
    }
}

void BLESDA::onDataWritten(const GattWriteCallbackParams *params) {
    mbed_tracef(TRACE_LEVEL_INFO,TRACE_GROUP_BLE,"Data Received");
    if (params->handle == getCharacteristicHandle()) {
    sda_over_ble_header buffer;
    memcpy(&buffer, params->data, START_DATA_BYTE);
    buffer.payload = (uint8_t*)malloc(buffer.frag_length);
    if(buffer.payload){
        memcpy(buffer.payload, &params->data[START_DATA_BYTE],buffer.frag_length);
    	}
    else {
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