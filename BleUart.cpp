#include "platform/Callback.h"
#include "UARTService.h"
#include "bleuart.h"

void BleUart::print_ble(const char* str) {
    _uart_service->writeString(str);
}

int BleUart::getc() {
    return _uart_service->_getc();
}
void BleUart::init_ble(){
    _uart_service = new UARTService(_ble_interface);
}

void BleUart::populateBuff() {
    int rxchar = getc();
    if(rxchar>-1){
        _RxBuff[_rxindex] = rxchar;
        _rxindex++;
    }
}

size_t BleUart::write(char* buff, size_t size){
    return _uart_service->write((const void*)buff, size);
}

size_t BleUart::read(char* buffOut, size_t buffSize){
    int rxchar = 0;
    for(uint8_t i=0; i< buffSize; i++){
        rxchar = getc();
       // printf("%d ", rxchar);
        if(rxchar > -1)
            buffOut[i]=rxchar;
        else
            return i;
    }
    return buffSize;
}

char* BleUart::getBuff(){
    return _RxBuff;
}

