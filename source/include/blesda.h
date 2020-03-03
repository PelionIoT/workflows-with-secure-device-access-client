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
#include "protocoltranslator.h"
#include "mbed.h"
#include "mbed_trace.h"

#define TRACE_GROUP             "ble"

#define SERIAL_NUM_INDEX        0
#define CONTROL_FRAME_INDEX     1
#define PACKET_LEN              2
#define UPPER_BYTE_REQ_INDEX    3
#define LOWER_BYTE_REQ_INDEX    4
#define START_DATA_BYTE         8
#define BLE_MTU_SIZE            231
#define SDA_DATA    1
#define SDA_ACK 2
// enum blep_err_code{
//     ERR_OK,
//     ERR_RESEND_REQ,
//     ERR_CRC,
//     ERR_MSG_NULL,
//     ERR_MEMORY_OVERFLOW,
// };
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
	uint8_t resv2;
    uint8_t resv3;
    uint16_t resv4;


	/* Payload */
    uint8_t* payload;
    /* CRC */
    uint8_t crc;
}sda_over_ble_header __attribute__((__packed__));



const uint8_t  UARTServiceBaseUUID[UUID::LENGTH_OF_LONG_UUID] = {
    0x6E, 0x40, 0x00, 0x00, 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};
const uint16_t UARTServiceShortUUID                 = 0x0001;
const uint16_t UARTServiceTXCharacteristicShortUUID = 0x0002;
const uint16_t UARTServiceRXCharacteristicShortUUID = 0x0003;
const uint8_t  UARTServiceUUID[UUID::LENGTH_OF_LONG_UUID] = {
    0x6E, 0x40, (uint8_t)(UARTServiceShortUUID >> 8), (uint8_t)(UARTServiceShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};
const uint8_t  UARTServiceUUID_reversed[UUID::LENGTH_OF_LONG_UUID] = {
    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
    0x93, 0xF3, 0xA3, 0xB5, (uint8_t)(UARTServiceShortUUID & 0xFF), (uint8_t)(UARTServiceShortUUID >> 8), 0x40, 0x6E
};
const uint8_t  UARTServiceTXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID] = {
    0x6E, 0x40, (uint8_t)(UARTServiceTXCharacteristicShortUUID >> 8), (uint8_t)(UARTServiceTXCharacteristicShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};
const uint8_t  UARTServiceRXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID] = {
    0x6E, 0x40, (uint8_t)(UARTServiceRXCharacteristicShortUUID >> 8), (uint8_t)(UARTServiceRXCharacteristicShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};


class BLESDA {
public:
    /** Maximum length of data (in bytes) that the UART service module can transmit to the peer. */
    static const unsigned BLE_UART_SERVICE_MAX_DATA_LEN = 244;//(BLE_GATT_MTU_SIZE_DEFAULT - 3);
public:
    /**
    * @param _ble
    *               BLE object for the underlying controller.
    */
    BLESDA(events::EventQueue &event_queue, BLE &_ble, char* endpointbuffer) :
        _event_queue(event_queue),
        ble(_ble),
        receiveBuffer(),
        sendBuffer(),
        sendBufferIndex(0),
        numBytesReceived(0),
        receiveBufferIndex(0),
        _endpointBuffer(endpointbuffer),
        _index(0),
        txCharacteristic(UARTServiceTXCharacteristicUUID, receiveBuffer, 1, BLE_UART_SERVICE_MAX_DATA_LEN,
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE),
        rxCharacteristic(UARTServiceRXCharacteristicUUID, sendBuffer, 1, BLE_UART_SERVICE_MAX_DATA_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        _total_req_size(0)
        {
            GattCharacteristic *charTable[] = {&txCharacteristic, &rxCharacteristic};
            GattService SDAService(UARTServiceUUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
            ble.gattServer().addService(SDAService);
            ble.gattServer().onDataWritten(this, &BLESDA::onDataWritten);
        }
    /**
     * Note: TX and RX characteristics are to be interpreted from the viewpoint of the GATT client using this service.
     */
    uint16_t getTXCharacteristicHandle();
    /**
     * Note: TX and RX characteristics are to be interpreted from the viewpoint of the GATT client using this service.
     */
    uint16_t getRXCharacteristicHandle();
    /*Need UUID of the service so that we can advertise that in the advertisement */
    const uint8_t* getUUID(){
        return UARTServiceUUID;
    }
    PTErr BLEHeaderTX(sda_over_ble_header* header, uint8_t len);
    /**
     * We attempt to collect bytes before pushing them to the UART RX
     * characteristic; writing to the RX characteristic then generates
     * notifications for the client. Updates made in quick succession to a
     * notification-generating characteristic result in data being buffered
     * in the Bluetooth stack as notifications are sent out. The stack has
     * its limits for this buffering - typically a small number under 10.
     * Collecting data into the sendBuffer buffer helps mitigate the rate of
     * updates. But we shouldn't buffer a large amount of data before updating
     * the characteristic, otherwise the client needs to turn around and make
     * a long read request; this is because notifications include only the first
     * 20 bytes of the updated data.
     *
     * @param  _buffer The received update.
     * @param  length Number of characters to be appended.
     * @return        Number of characters appended to the rxCharacteristic.
     */
    size_t write(uint8_t *_buffer, size_t length);
    /**
     * Helper function to write out strings.
     * @param  str The received string.
     * @return     Number of characters appended to the rxCharacteristic.
     */
    size_t writeString(const char *str);
    /**
     * Flush sendBuffer, i.e., forcefully write its contents to the UART RX
     * characteristic even if the buffer is not full.
     */
    void flush(uint8_t* buff, uint8_t length);    /**
     * Override for Stream::_putc().
     * @param  c
     *         This function writes the character c, cast to an unsigned char, to stream.
     * @return
     *     The character written as an unsigned char cast to an int or EOF on error.
     */
    /*send ack to the mobile*/
    void sendACK(_sda_over_ble_header* buffer);
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
    PTErr ProcessBuffer(sda_over_ble_header* frag_sda);
    PTErr sda_fragment_datagram(uint8_t* sda_payload, uint16_t payloadsize);
    PTErr processInputBuffer(uint8_t* msg_in, uint8_t bytesRecieved);

private:
    //Thread t;

    BLE                 &ble;                                           //BLe instance
    uint16_t             _index;
    uint8_t              receiveBuffer[BLE_UART_SERVICE_MAX_DATA_LEN]; /**   The local buffer into which we receive
                                                                       *    inbound data before forwarding it to the
                                                                       *    application.     */
    uint8_t              sendBuffer[BLE_UART_SERVICE_MAX_DATA_LEN];      /** The local buffer into which outbound data is
                                                                        *   accumulated before being pushed to the
                                                                        *   rxCharacteristic. */
    char*               _endpointBuffer;                            /*  The local buffer that contains the endpoint of
                                                                        the device.       */
    events::EventQueue &_event_queue;
    uint8_t             sendBufferIndex;
    uint8_t             numBytesReceived;
    uint8_t             receiveBufferIndex;
    uint16_t            _total_req_size;
    GattCharacteristic  txCharacteristic;                               /**< From the point of view of the external client, this is the characteristic
                                                                        *   they'd write into in order to communicate with this application. */
    GattCharacteristic  rxCharacteristic;                               /**< From the point of view of the external client, this is the characteristic
                                                                        *   they'd read from in order to receive the bytes transmitted by this
                                                                        *   application. */
};
#endif