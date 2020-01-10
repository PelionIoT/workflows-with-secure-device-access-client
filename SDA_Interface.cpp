#include "sda_interface.h"
#include <stdio.h>


int SDAInterface::sdaProcess(uint8_t* buffer, uint8_t buffSize ){
    printf("Buffer - %s Size - %d \r\n", buffer, buffSize);
}