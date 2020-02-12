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
#include "include/blesda.h"
using mbed::callback;


uint8_t request[54] = {0,SDA_DATA|0<<3|1<<4,46,0,46,0,0,0,109, 98, 101, 100, 100, 98, 97, 112, 0, 0, 0, 2, 129, 1, 172, 56, 120, 63, 106, 59, 47, 227, 181, 121, 113, 141,
                            107, 168, 73, 58, 69, 109, 128, 6, 101, 178, 67, 58, 14, 124, 130, 60, 255, 144, 166, 3};
uint8_t g_seqnum = 0;
uint16_t BLESDA::getTXCharacteristicHandle() {
    return txCharacteristic.getValueAttribute().getHandle();
}
/**
 * Note: TX and RX characteristics are to be interpreted from the viewpoint of the GATT client using this service.
 */
uint16_t BLESDA::getRXCharacteristicHandle() {
    return rxCharacteristic.getValueAttribute().getHandle();
}
size_t BLESDA::write(const void *_buffer, size_t length) {
    size_t         origLength = length;
    const uint8_t *buffer     = static_cast<const uint8_t *>(_buffer);
    printf("1x\r\n");

    if (ble.getGapState().connected) {
        printf("2\r\n");
        unsigned bufferIndex = 0;
        while (length) {
            printf("3\r\n");
            unsigned bytesRemainingInSendBuffer = BLE_UART_SERVICE_MAX_DATA_LEN - sendBufferIndex;
            unsigned bytesToCopy                = (length < bytesRemainingInSendBuffer) ? length : bytesRemainingInSendBuffer;

            /* Copy bytes into sendBuffer. */
            memcpy(&sendBuffer[sendBufferIndex], &buffer[bufferIndex], bytesToCopy);
            length          -= bytesToCopy;
            sendBufferIndex += bytesToCopy;
            bufferIndex     += bytesToCopy;

            /* Have we collected enough? */
            if ((sendBufferIndex == BLE_UART_SERVICE_MAX_DATA_LEN) ||
                // (sendBuffer[sendBufferIndex - 1] == '\r')          ||
                (sendBuffer[sendBufferIndex - 1] == '\n')) {
                printf("4\r\n");
                ble.gattServer().write(getRXCharacteristicHandle(), static_cast<const uint8_t *>(sendBuffer), sendBufferIndex);
                sendBufferIndex = 0;
            }
        }
     }
    return origLength;
}

size_t BLESDA::writeString(const char *str) {
    return write(str, strlen(str));
}
void BLESDA::flush() {
    if (ble.getGapState().connected) {
        if (sendBufferIndex != 0) {
            ble.gattServer().write(getRXCharacteristicHandle(), static_cast<const uint8_t *>(sendBuffer), sendBufferIndex);
            sendBufferIndex = 0;
        }
    }
}
int BLESDA::_putc(int c) {
    return (write(&c, 1) == 1) ? 1 : EOF;
}
int BLESDA::_getc() {
    if (receiveBufferIndex == numBytesReceived) {
        return EOF;
    }
    return receiveBuffer[receiveBufferIndex++];
}

PTErr BLESDA::BLEHeaderTX(_sda_over_ble_header* header, uint8_t len){
    printf("SeqNum - %d type - %d MF - %d Fragment Num - %d frag_length - %d total size %ld first length %d second length %d  \r\n", header->seq_num,
    header->type, header->more_frag, header->frag_num, header->frag_length,header->length, header->length & 0xff00 >> 8,header->length & 0x00ff);
    uint8_t transmit_data_len = len+START_DATA_BYTE;
    uint8_t* msg = (uint8_t*)malloc(transmit_data_len);
    msg[0] = header->seq_num;
    msg[1] = (header->type|(header->more_frag<<2)|(header->frag_num<<3));
    msg[2] = (header->length & 0xff00) >> 8;
    msg[3] = (header->length & 0x00ff);
    msg[4] = header->frag_length;
    memcpy(&msg[8], header->payload, len);
    printf("---------Header----------\r\n");
    printf("Transmitting data to BLE - %d\r\n",transmit_data_len);
    write(msg, transmit_data_len);
    flush();
    printf("\r\n");
    if(msg){
        free(msg);
    }
}


//function that process buffer according to BLEP protocol
PTErr BLESDA::sda_fragment_datagram(uint8_t* sda_payload, uint16_t payloadsize)
{
    uint8_t fragmentStartOffset=0;
    uint8_t frag_num=0;
    uint16_t totalpayloadsize=payloadsize;
    //Calculate number of fragment in order to be transmitted
    uint8_t num_frag = (payloadsize/BLE_MTU_SIZE)+1;
    while(num_frag)
    {
        sda_over_ble_header frag_sda ={0};
        frag_sda.seq_num = g_seqnum;
        g_seqnum = g_seqnum+1;
        frag_sda.type = SDA_DATA;
        frag_sda.more_frag = (num_frag==1)?0:1;
        frag_sda.frag_num=(++frag_num);
        frag_sda.length=totalpayloadsize;
        frag_sda.frag_length=(payloadsize<BLE_MTU_SIZE)? payloadsize : BLE_MTU_SIZE;
        frag_sda.payload = (uint8_t*) malloc(BLE_MTU_SIZE);
        if(frag_sda.payload)
            {
                memcpy(&frag_sda.payload[0],&sda_payload[fragmentStartOffset],BLE_MTU_SIZE);
            }
        if(payloadsize<BLE_MTU_SIZE) {
            fragmentStartOffset = 0;
        }
        else{
            fragmentStartOffset+=BLE_MTU_SIZE;
        }
    //Transmit it to BLE
    PTErr status = BLEHeaderTX(&frag_sda, payloadsize);
    if(status != PT_ERR_OK){
        return status;
    }
    payloadsize -= BLE_MTU_SIZE;
    //Wait for Tx Done
    //free allocated memory
        if(frag_sda.payload)
        {
            free(frag_sda.payload);
        }
    num_frag--;
    printf("Num frag %d\r\n",num_frag);
    }
    return PT_ERR_OK;
    //free(sda_payload);
 }
PTErr BLESDA::ProcessBuffer(sda_over_ble_header* frag_sda){
    printf("Processing buffer\r\n");
    uint8_t* msg_to_sda;
    uint8_t* response = (uint8_t*)malloc(60*sizeof(uint8_t));
    uint8_t response_size = 60;
    uint16_t sda_response_size=0;
    if(frag_sda->frag_num == 1){
        msg_to_sda = (uint8_t*)malloc(frag_sda->frag_length*sizeof(uint8_t));
    }
    memcpy(&msg_to_sda[_index],frag_sda->payload, frag_sda->frag_length);

    if(frag_sda->more_frag == 1){
       _index +=frag_sda->frag_length;
    }
    else{
        printf("Sending buffer\r\n");
        for(int i = 0 ; i < frag_sda->frag_length;i++)
            printf("%d ",msg_to_sda[i]);
        ProtocolTranslator* protocoltranslator = new ProtocolTranslator(msg_to_sda);
        if(protocoltranslator->init(response,response_size,&sda_response_size)!=PT_ERR_OK){
            return PT_ERR_PROCESS_REQ;
        }
        delete protocoltranslator;
        return sda_fragment_datagram(response, sda_response_size);
        _index = 0;
    }
    if(response){
        free(response);
    }
}
PTErr BLESDA::processInputBuffer(uint8_t* msg_in) {
    if(msg_in == NULL) {
        return PT_ERR_EMPTY_MSG;
    }
    sda_over_ble_header buffer;
    buffer.seq_num = msg_in[0];
    buffer.type = (msg_in[1] & 0x03);
    buffer.more_frag = ((msg_in[1] & 0x04)>>2);
    buffer.frag_num = ((msg_in[1]& 0x38)>>3);
    buffer.length = (msg_in[2]<< 8| (msg_in[3]));
    buffer.frag_length = msg_in[4];
    uint16_t total_req_size = buffer.length;
    buffer.payload = (uint8_t*)malloc(buffer.frag_length);
    memcpy(buffer.payload, &msg_in[8],buffer.frag_length);
    return ProcessBuffer(&buffer);
}
void BLESDA::onDataWritten(const GattWriteCallbackParams *params) {
    printf("Data Received \r\n");
    if (params->handle == getTXCharacteristicHandle()) {
        uint16_t bytesRead = params->len;
        if (bytesRead <= BLE_UART_SERVICE_MAX_DATA_LEN) {
            numBytesReceived   = bytesRead;
            receiveBufferIndex = 0;
            memcpy(receiveBuffer, params->data, numBytesReceived);
        }
        processInputBuffer(request);
    }
}