#include "blesda.h"
#ifndef _PROTOCOL_TRANSLATOR_H_
#define _PROTOCOL_TRANSLATOR_H_

#define FTCD_MSG_HEADER_TOKEN_SDA { 0x6d, 0x62, 0x65, 0x64, 0x64, 0x62, 0x61, 0x70 }
#define FTCD_MSG_HEADER_TOKEN_SIZE_BYTES 8

class ProtocolTranslator{
    public:
        ProtocolTranslator(uint8_t* request):
        _message_header(FTCD_MSG_HEADER_TOKEN_SDA),_index(0) {};
        void init();
        bool is_token_detected();
        uint32_t read_message_size();
        bool read_message(uint8_t* message, uint32_t message_size);
        bool read_message_signature(uint8_t* message, size_t message_size);

    private:
        int _index;
        size_t _read(uint8_t* buffer, size_t message_size);
        size_t _write(uint8_t* buffer, size_t message_size);
        uint8_t* _buffer;
        uint8_t _message_header[FTCD_MSG_HEADER_TOKEN_SIZE_BYTES];
        bool _use_signature;
};

#endif
