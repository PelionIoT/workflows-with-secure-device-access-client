#include "platform/Callback.h"
#include "UARTService.h"
#include "bleuart.h"

using mbed::callback;

void BleUart::print_ble(const char* str) {
    _uart_service->writeString(str);
}

int BleUart::getc() {
    return _uart_service->_getc();
}
void BleUart::init_ble(){
    _uart_service = new UARTService(_ble_interface);
}

size_t BleUart::write(char* buff, size_t size){
    return _uart_service->write((const void*)buff, size);
}
char* BleUart::readBuff() {
    char buffOut[18]= "";
    int rxchar = 0;
    for(int i=0; i < 18; i++) {
        rxchar = getc();
        if(rxchar > -1)
            buffOut[i]=rxchar;
    }
    print_ble(buffOut);
    return buffOut;
}

size_t BleUart::read(char* buffOut, size_t buffSize){
    int rxchar = 0;
    for(uint8_t i=0; i< buffSize; i++){
        rxchar = getc();
       // printf("%d", rxchar);
        if(rxchar > -1){
                buffOut[i]=rxchar;
                //printf("%d", rxchar);
        }
        else
            return i;
    }
    return buffSize;
}

char* BleUart::getBuff(){
    return _RxBuff;
}

