#ifndef __SDA_INTERFACE_H__
#define __SDA_INTERFACE_H__
#endif
#include <stdint.h>
class SDAInterface{
    public:
        SDAInterface();
        int sdaProcess(uint8_t* buffer, uint8_t buffSize );
};