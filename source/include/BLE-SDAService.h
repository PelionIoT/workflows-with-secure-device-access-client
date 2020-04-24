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
#ifndef _BLE_SDA_SERVICE_H_
#define _BLE_SDA_SERVICE_H_
#include "frag_buff.h"
#include "events/EventQueue.h"
#include "ble/UUID.h"
#include "ble/BLE.h"
#include "ble/pal/Deprecated.h"
#include "SDAOperation.h"
#include "mbed_trace.h"
#include "mbed.h"
#include "./../../ble-config-uuid.h"

#define TRACE_GROUP                 "BLESDA"
#define UPPER_BYTE_REQ_INDEX            3
#define LOWER_BYTE_REQ_INDEX            4
#define START_DATA_BYTE                 8
#define SDA_DATA                        1
#define SDA_ACK                         3
#define SDA_REQ                         2

class BLESDA {
public:
    /** Maximum length of data (in bytes) that the UART service module can transmit to the peer. */
    static const unsigned BLE_UART_SERVICE_MAX_DATA_LEN = 244;
public:
    BLESDA(events::EventQueue &event_queue, BLE &_ble, const char* endpointbuffer) :
        _event_queue(event_queue),
        ble(_ble),
        _endpointBuffer(endpointbuffer),
        sdaCharacteristic(CharacteristicUUID, NULL, 1, BLE_UART_SERVICE_MAX_DATA_LEN,
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE |
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
        {
            GattCharacteristic *charTable[] = {&sdaCharacteristic};
            GattService SDAService(ServiceUUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
            ble.gattServer().addService(SDAService);
            ble.gattServer().onDataWritten(this, &BLESDA::onDataWritten);
        }

    uint16_t getCharacteristicHandle();

    const uint8_t* getUUID(){   //small method that returns UUID of the service.
        return ServiceUUID;
    }

    sda_protocol_error_t BLETX(Frag_buff* header, uint8_t len);
    size_t write(uint8_t *_buffer, uint8_t length);
    uint8_t* getRecievedBuffer();
    void onDataWritten(const GattWriteCallbackParams *params);
    sda_protocol_error_t ProcessBuffer(Frag_buff* frag_sda);
    sda_protocol_error_t sda_fragment_datagram(uint8_t* sda_payload, uint16_t payloadsize, uint8_t type);
    sda_protocol_error_t processInputBuffer(uint8_t* msg_in, uint16_t bytesRecieved);
    sda_protocol_error_t processRequest(Frag_buff* frag_sda);

private:
    char* getEndpoint();
    events::EventQueue &_event_queue;
    BLE                 &ble;
    const char*         _endpointBuffer;
    uint8_t*            msg_to_sda = NULL;
    GattCharacteristic  sdaCharacteristic;
};
#endif