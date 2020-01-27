#include "include/blesda.h"


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
 * a long read request; this is because notifications include of the updated data.
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

void BLESDA::onDataRead(const GattReadCallbackParams *params){
    printf("Data Read Request complete \r\n");
}

//function that process buffer acorfding to BLEP protocol
uint8_t* BLESDA::processBuffer(uint8_t* buffer) {
    const uint8_t start_data = 15;
    uint8_t s_num = buffer[0];   // Serial num
    uint8_t c_frame = buffer[1]; // Control byte, it has two fields, higher octet represents MF(More Fragment) bit and
                                 // lower bit represents fragment number.
    uint16_t req_size = buffer[2] << 16+buffer[3]; // right now it's two bytes but it will increase more fields, it should be 4 bytes.
    uint8_t* req = (uint8_t*)malloc(req_size);

    for(uint8_t i =  start_data; i < req_size; i++){
        req[i-start_data]=buffer[i];
    }

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
        if(strncmp((const char*)receiveBuffer,"getEndpoint",numBytesReceived)==0){
            writeString((char*)_endpointBuffer);
            flush();
            receiveBufferIndex=0;
            memset(receiveBuffer,0,numBytesReceived);
            return;
        }
        else{
            writeString((char*)receiveBuffer);
            flush();
            receiveBufferIndex=0;
            memset(receiveBuffer,0,numBytesReceived);
        }
        _protocoltranslator = new ProtocolTranslator(receiveBuffer);
        _protocoltranslator->init();
    }
}

