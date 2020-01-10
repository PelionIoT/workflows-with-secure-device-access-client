#include "blesda.h"

uint16_t BLESDA::getTXCharacteristicHandle() {
    return txCharacteristic.getValueAttribute().getHandle();
}

/**
 * Note: TX and RX characteristics are to be interpreted from the viewpoint of the GATT client using this service.
 */
uint16_t BLESDA::getRXCharacteristicHandle() {
    return rxCharacteristic.getValueAttribute().getHandle();
}

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
size_t BLESDA::write(const void *_buffer, size_t length) {
    size_t         origLength = length;
    const uint8_t *buffer     = static_cast<const uint8_t *>(_buffer);

    if (ble.getGapState().connected) {
        unsigned bufferIndex = 0;
        while (length) {
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

void BLESDA::onDataWritten(const GattWriteCallbackParams *params) {
    printf("Inside On Data Written \r\n");
    if (params->handle == getTXCharacteristicHandle()) {
        uint16_t bytesRead = params->len;
        if (bytesRead <= BLE_UART_SERVICE_MAX_DATA_LEN) {
            numBytesReceived   = bytesRead;
            receiveBufferIndex = 0;
            memcpy(receiveBuffer, params->data, numBytesReceived);
        }
        printf("On data Written %s\r\n", receiveBuffer);
        printf("Bytes Read - %d\r\n", bytesRead);
        _sda_interface->sdaProcess(receiveBuffer, bytesRead);
    }
}

