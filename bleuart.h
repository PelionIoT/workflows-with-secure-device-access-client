#include "UARTService.h"
#include "events/EventQueue.h"
#include "platform/Callback.h"

#ifndef BLEUART_H_
#define BLEUART_H_


class BleUart{
    typedef BleUart Self;
    public:
        BleUart(BLE &ble_interface):
            _ble_interface(ble_interface),
            _RxBuff(""),
            _rxindex(0) {

            };
        ~BleUart();
        void init_ble();
        void clear_ble();
        char* readBuff();
        size_t read(char* buffOut, size_t buffSize);
        size_t write(char* buff, size_t size);

    private:
        int getc();
        int putc(int c);
        void print_ble(const char* str);
        int getIndex();
        char* getBuff();
        void populateBuff();
        UARTService* _uart_service;
        char _RxBuff[50];
        uint8_t _rxindex = 0;
        BLE &_ble_interface;
};
#endif