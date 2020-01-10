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

#ifndef __BLE_SDA_SERVICE_H_
#define __BLE_SDA_SERVICE_H_
#endif

#include "sda_interface.h"
#include "ble/UUID.h"
#include "ble/BLE.h"
#include "ble/pal/Deprecated.h"

extern const uint8_t  UARTServiceBaseUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint16_t UARTServiceShortUUID;
extern const uint16_t UARTServiceTXCharacteristicShortUUID;
extern const uint16_t UARTServiceRXCharacteristicShortUUID;

extern const uint8_t  UARTServiceUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint8_t  UARTServiceUUID_reversed[UUID::LENGTH_OF_LONG_UUID];

extern const uint8_t  UARTServiceTXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint8_t  UARTServiceRXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID];


class BLESDA {
public:
    /** Maximum length of data (in bytes) that the UART service module can transmit to the peer. */
    static const unsigned BLE_UART_SERVICE_MAX_DATA_LEN = 244;//(BLE_GATT_MTU_SIZE_DEFAULT - 3);

public:

    /**
    * @param _ble
    *               BLE object for the underlying controller.
    */
    BLESDA(BLE &_ble) :
        ble(_ble),
        receiveBuffer(),
        sendBuffer(),
        sendBufferIndex(0),
        numBytesReceived(0),
        receiveBufferIndex(0),
        txCharacteristic(UARTServiceTXCharacteristicUUID, receiveBuffer, 1, BLE_UART_SERVICE_MAX_DATA_LEN,
                         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE),
        rxCharacteristic(UARTServiceRXCharacteristicUUID, sendBuffer, 1, BLE_UART_SERVICE_MAX_DATA_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        epoint(UARTServiceRXCharacteristicUUID, sendBuffer, 1, BLE_UART_SERVICE_MAX_DATA_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ)
        {
        GattCharacteristic *charTable[] = {&txCharacteristic, &rxCharacteristic, &epoint};
        GattService SDAService(UARTServiceUUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));

        ble.addService(SDAService);
        ble.onDataWritten(this, &BLESDA::onDataWritten);
    }

    /**
     * Note: TX and RX characteristics are to be interpreted from the viewpoint of the GATT client using this service.
     */
    uint16_t getTXCharacteristicHandle();
    /**
     * Note: TX and RX characteristics are to be interpreted from the viewpoint of the GATT client using this service.
     */
    uint16_t getRXCharacteristicHandle();

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
    size_t write(const void *_buffer, size_t length);
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
    void flush();

    /**
     * Override for Stream::_putc().
     * @param  c
     *         This function writes the character c, cast to an unsigned char, to stream.
     * @return
     *     The character written as an unsigned char cast to an int or EOF on error.
     */
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

private:
    BLE                &ble;

    uint8_t             receiveBuffer[BLE_UART_SERVICE_MAX_DATA_LEN]; /**< The local buffer into which we receive
                                                                       *   inbound data before forwarding it to the
                                                                       *   application. */

    uint8_t             sendBuffer[BLE_UART_SERVICE_MAX_DATA_LEN];    /**< The local buffer into which outbound data is
                                                                       *   accumulated before being pushed to the
                                                                       *   rxCharacteristic. */
    uint8_t             endpointBuffer[BLE_UART_SERVICE_MAX_DATA_LEN];
    uint8_t             sendBufferIndex;
    uint8_t             numBytesReceived;
    uint8_t             receiveBufferIndex;
    SDAInterface*       _sda_interface;

    GattCharacteristic  txCharacteristic; /**< From the point of view of the external client, this is the characteristic
                                           *   they'd write into in order to communicate with this application. */
    GattCharacteristic  rxCharacteristic; /**< From the point of view of the external client, this is the characteristic
                                           *   they'd read from in order to receive the bytes transmitted by this
                                           *   application. */
    GattCharacteristic epoint; // characteristic  that returns the endpoint
};

