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
#include "events/EventQueue.h"
#include "ble/UUID.h"
#include "ble/BLE.h"
#include "ble/pal/Deprecated.h"
#include "SDAOperation.h"
#include "mbed_trace.h"
#include "mbed.h"
#include "./../../ble-config-uuid.h"

#define TRACE_GROUP_BLE             "BLESDA"
#define UPPER_BYTE_REQ_INDEX    3
#define LOWER_BYTE_REQ_INDEX    4
#define START_DATA_BYTE         8
#define SDA_DATA                1
#define SDA_ACK                 3
#define SDA_REQ                 2

typedef struct _sda_over_ble_header {
	 /* Frame sequence number(1byte) */
	uint8_t seq_num;
    /* control frame field 12byte) */
	uint8_t type:2;
    uint8_t more_frag:1;
	uint8_t frag_num:3;
        /* Frag length (1byte) */
    uint8_t frag_length;
    /* Total length (2byte) */
    uint16_t length;
    uint8_t resv1:2;
	uint8_t resv2;
    uint8_t resv3;
    uint16_t resv4;


	/* Payload */
    uint8_t* payload;
}__attribute__((packed)) sda_over_ble_header;

class BLESDA {
public:
    /** Maximum length of data (in bytes) that the UART service module can transmit to the peer. */
    static const unsigned BLE_UART_SERVICE_MAX_DATA_LEN = 244;//(BLE_GATT_MTU_SIZE_DEFAULT - 3);
public:
    /**
    * @param _ble
    *               BLE object for the underlying controller.
    */
    BLESDA(events::EventQueue &event_queue, BLE &_ble, const char* endpointbuffer) :
        _event_queue(event_queue),
        ble(_ble),
        _endpointBuffer(endpointbuffer),
        sdaCharacteristic(CharacteristicUUID, NULL, 1, BLE_UART_SERVICE_MAX_DATA_LEN,
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE
                         |GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
        {
            GattCharacteristic *charTable[] = {&sdaCharacteristic};
            GattService SDAService(ServiceUUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
            ble.gattServer().addService(SDAService);
            ble.gattServer().onDataWritten(this, &BLESDA::onDataWritten);
        }
    /**
     * Note: TX and RX characteristics are to be interpreted from the viewpoint of the GATT client using this service.
     */
    uint16_t getCharacteristicHandle();
    /**
     * Note: TX and RX characteristics are to be interpreted from the viewpoint of the GATT client using this service.
     */
    //uint16_t getCharacteristicHandle();
    /*Need UUID of the service so that we can advertise that in the advertisement */
    const uint8_t* getUUID(){
        return ServiceUUID;
    }

    sda_protocol_error_t BLETX(sda_over_ble_header* header, uint8_t len);

    size_t write(uint8_t *_buffer, uint8_t length);

    int _putc(int c);
    /**
     * Override for Stream::_getc().
     * @return
     *     The character read.
     */
    int _getc();
    uint8_t* getRecievedBuffer();
    /**
     * This callback allows the UART service to receive updates to the
     * txCharacteristic. The application should forward the call to this
     * function from the global onDataWritten() callback handler; if that's
     * not used, this method can be used as a callback directly.
     */
    void onDataWritten(const GattWriteCallbackParams *params);
    /*
    This callback allows the connecting device to fetch the endpoint of the device so that it can proceed further.
    */
    void onDataRead(const GattReadCallbackParams *params);
    sda_protocol_error_t ProcessBuffer(sda_over_ble_header* frag_sda);
    sda_protocol_error_t sda_fragment_datagram(uint8_t* sda_payload, uint16_t payloadsize, uint8_t type);
    sda_protocol_error_t processInputBuffer(uint8_t* msg_in, uint16_t bytesRecieved);
    sda_protocol_error_t processRequest(sda_over_ble_header* frag_sda);

private:
    //Thread t;
    char* getEndpoint();
    events::EventQueue &_event_queue;
    BLE                 &ble;                                           //  BLe instance
    const char*         _endpointBuffer;                                /*  The local buffer that contains the endpoint of
                                                                        the device.       */
    uint8_t*            msg_to_sda = NULL;
    GattCharacteristic  sdaCharacteristic;                               /**< From the point of view of the external client, this is the characteristic
                                                                        *   they'd write into in order to communicate with this application. */
    // GattCharacteristic  Characteristic;                               /**< From the point of view of the external client, this is the characteristic
    //                                                                     *   they'd read from in order to receive the bytes transmitted by this
    //                                                                     *   application. */
};
#endif