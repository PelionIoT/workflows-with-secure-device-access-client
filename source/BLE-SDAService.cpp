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
#define TRACE_GROUP "ble"


uint8_t request[54] = {0,SDA_DATA|0<<2|1<<3,46,0,46,0,0,0,109, 98, 101, 100, 100, 98, 97, 112, 0, 0, 0, 2, 129, 1, 172, 56, 120, 63, 106, 59, 47, 227, 181, 121, 113, 141,
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
size_t BLESDA::write(uint8_t *_buffer, size_t length) {
    printf("N%s %d\n",__FUNCTION__,length);


    size_t         origLength = length;
    const uint8_t *buffer     = static_cast<const uint8_t *>(_buffer);
    for(int i = 8 ; i < length; i++)
        printf("%d ",_buffer[i]);
  //  printf("1x\r\n");

    if (ble.getGapState().connected) {
    //    printf("2\r\n");
        unsigned bufferIndex = 0;
        while (length) {
      //      printf("3\r\n");
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
        //        printf("4\r\n");
                ble.gattServer().write(getRXCharacteristicHandle(), static_cast<const uint8_t *>(sendBuffer), sendBufferIndex);
                sendBufferIndex = 0;
            }
        }
     }
    return origLength;
}

// size_t BLESDA::writeString(const char *str) {
//     return write(str, strlen(str));
// }
void BLESDA::flush() {
    if (ble.getGapState().connected) {
        if (sendBufferIndex != 0) {
            ble.gattServer().write(getRXCharacteristicHandle(), static_cast<const uint8_t *>(sendBuffer), sendBufferIndex);
            sendBufferIndex = 0;
        }
    }
}
// int BLESDA::_putc(int c) {
//     return (write(&c, 1) == 1) ? 1 : EOF;
// }
// int BLESDA::_getc() {
//     if (receiveBufferIndex == numBytesReceived) {
//         return EOF;
//     }
//     return receiveBuffer[receiveBufferIndex++];
// }

PTErr BLESDA::BLEHeaderTX(_sda_over_ble_header* header, uint8_t len){
    //printf("SeqNum - %d type - %d MF - %d Fragment Num - %d frag_length - %d total size %ld first length %d second length %d  \r\n", header->seq_num,
    //header->type, header->more_frag, header->frag_num, header->frag_length,header->length, header->length & 0xff00 >> 8,header->length & 0x00ff);
    printf("Seq Num: %d", header->seq_num);
    uint8_t transmit_data_len = len+START_DATA_BYTE+1;
    uint8_t* msg = (uint8_t*)malloc(transmit_data_len*sizeof(uint8_t));
    msg[0] = header->seq_num;
    msg[1] = (header->type|(header->more_frag<<2)|(header->frag_num<<3));
    msg[2] = (header->frag_length);
    msg[3] = (header->length & 0xff00>>8);
    msg[4] = (header->length & 0x00ff);
    memcpy(&msg[START_DATA_BYTE], header->payload, len);
  //  msg[transmit_data_len-1] = 32;
   // printf("---------Header----------\r\n");
   // printf("Transmitting data to BLE - %d\r\n",transmit_data_len);
    write(msg, transmit_data_len);
    flush();
    wait_ms(500);
    free(msg);
    //return PT_ERR_OK;
}


//function that process buffer according to BLEP protocol
PTErr BLESDA::sda_fragment_datagram(uint8_t* sda_payload, uint16_t payloadsize)
{
    printf("%s %d\n",__FUNCTION__,payloadsize);
    uint8_t fragmentStartOffset=0;
    static uint8_t frag_num=0;
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
        frag_sda.payload = (uint8_t*) malloc(frag_sda.frag_length);
        if(frag_sda.payload!=NULL)
            {
                printf("H\n");
              //  memcpy(&frag_sda.payload[0],&sda_payload[fragmentStartOffset],frag_sda.frag_length);
              for(int i=0;i<frag_sda.frag_length;i++)
              {
                    frag_sda.payload[i]=sda_payload[fragmentStartOffset+i];
                    printf("%d ",frag_sda.payload[i]);
              }
            }
            else{
                mbed_tracef(TRACE_LEVEL_ERROR,TRACE_GROUP,"can not create payload to send to ble");
            }
        if(payloadsize<BLE_MTU_SIZE) {
            fragmentStartOffset = 0;
        }
        else{
            fragmentStartOffset+=frag_sda.frag_length;
        }
    //Transmit it to BLE
   //     BLEHeaderTX(&frag_sda, payloadsize);
      num_frag--;
      //write((uint8_t*)&frag_sda, frag_sda.frag_length+8+1);
      BLEHeaderTX(&frag_sda,frag_sda.frag_length);
    // if(status != PT_ERR_OK){
    //     return status;
    // }
        payloadsize -= BLE_MTU_SIZE;
    //Wait for Tx Done
    //free allocated memory
        free(frag_sda.payload);
    }
  //  free(sda_payload);
    return PT_ERR_OK;
    //free(sda_payload);
 }

// uint8_t * my_realloc(int *p, int total_memory_alloc)
//  {
//      if(p==NULL)
//      {
//          return ((uint8_t*)malloc(total_memory_alloc));
//      }
//     else
//     {
//         return ((uint8_t*)realloc(p,total_memory_alloc));
//     }      
//  }
 #define response_size 231
PTErr BLESDA::ProcessBuffer(sda_over_ble_header* frag_sda){
    mbed_tracef(TRACE_LEVEL_INFO, TRACE_GROUP,"Processing buffer\r\n");
    static uint8_t *msg_to_sda = NULL;
    static uint16_t idx=0;
    //uint8_t response_size = 231;
    uint8_t response[response_size]={0};
    uint16_t sda_response_size=0;
    if(frag_sda->frag_num == 1){
        msg_to_sda = (uint8_t*)malloc(frag_sda->length/**sizeof(uint8_t)*/);
        if(msg_to_sda==NULL){
            mbed_tracef(TRACE_LEVEL_ERROR,TRACE_GROUP,"Could not allocate memoryfor message to sda");
            return PT_ERR_MSG;
        }
    }
    memcpy(&msg_to_sda[idx],frag_sda->payload, frag_sda->frag_length);

    if(frag_sda->more_frag == 1){
       idx +=frag_sda->frag_length;
    }
    else{
        idx = 0;
        mbed_tracef(TRACE_LEVEL_INFO,TRACE_GROUP,"Sending buffer to SDA\r\n");
        ProtocolTranslator* protocoltranslator = new ProtocolTranslator(msg_to_sda);
        PTErr status = protocoltranslator->init(response,response_size,&sda_response_size);
        printf("sda :%d",sda_response_size);
        for(int i = 0 ; i < sda_response_size; i++)
            printf("%d ", response[i]);
        if(status!=PT_ERR_OK){
            return status;
        }
    delete protocoltranslator;
    if(msg_to_sda!=NULL){
        free(msg_to_sda);
    }
    return sda_fragment_datagram(response, sda_response_size); //try write here directly
    }
}
// PTErr BLESDA::processInputBuffer(uint8_t* msg_in, uint8_t numBytesReceived) {
//     if(msg_in == NULL) {
//         return PT_ERR_EMPTY_MSG;
//     }
//     sda_over_ble_header buffer = {0};
//     memcpy((uint8_t *)&buffer,(uint8_t *)&msg_in[0],8);
//     //buffer->payload = (uint8_t*)malloc(msg[2]);
//     // if(!buffer->payload){
//     //     mbed_tracef(TRACE_ACTIVE_LEVEL_ERROR, TRACE_GROUP, "Could not allocate memory");
//     // }
//     //buffer = (sda_over_ble_header *)msg_in;
//     #if vishal
//     buffer.seq_num = msg_in[0];
//     buffer.type = (msg_in[1] & 0x03);
//     buffer.more_frag = ((msg_in[1] & 0x04)>>2);
//     buffer.frag_num = ((msg_in[1]& 0x38)>>3);
//     buffer.frag_length = msg_in[2];
//     buffer.length = (msg_in[3]<< 8| (msg_in[4]));
//     #endif

//     buffer.payload = (uint8_t*)malloc(buffer.frag_length*sizeof(uint8_t));
//     if(buffer.payload==NULL){
//             mbed_tracef(TRACE_ACTIVE_LEVEL_ERROR, TRACE_GROUP, "Could not allocate memory");
//     }
//     memcpy(&(buffer.payload[0]), &msg_in[8],buffer.frag_length);
//     PTErr status = ProcessBuffer(&buffer);
//   //  if(status!=PT_ERR_OK)
//    {
//         free(buffer.payload);
//         return status;
//     }
// }
void BLESDA::onDataWritten(const GattWriteCallbackParams *params) {
    printf("Data Received \r\n");
    sda_over_ble_header buffer = {0};  
    if (params->handle == getTXCharacteristicHandle()) {
        uint16_t bytesRead = params->len;
        if (bytesRead <= BLE_UART_SERVICE_MAX_DATA_LEN) {
            numBytesReceived   = bytesRead;
            receiveBufferIndex = 0;
             memcpy((uint8_t *)&buffer,(uint8_t *)&params->data[0],8);
            buffer.payload = (uint8_t*)malloc(buffer.frag_length*sizeof(uint8_t));
            if(buffer.payload==NULL){
                    mbed_tracef(TRACE_ACTIVE_LEVEL_ERROR, TRACE_GROUP, "Could not allocate memory");
                    return;
            }
            for(int i = 0 ; i < 8 ; i++)
                printf("%d ",params->data[i]);
            printf("F:%d T:%d %d %d \n",buffer.frag_length,buffer.length,sizeof(buffer.payload),sizeof(buffer.resv4));
            memcpy(&(buffer.payload[0]), &params->data[8],buffer.frag_length);
        }
        PTErr status = ProcessBuffer(&buffer);
         if(status!=PT_ERR_OK)
        {
            mbed_tracef(TRACE_ACTIVE_LEVEL_ERROR, TRACE_GROUP, "Unable to process SDA buffer");
        }
        free(buffer.payload);
    }
}