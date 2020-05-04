
#ifndef __BLE_CONFIG_UUID__
#define __BLE_CONFIG_UUID__

#include<stdint.h>

#define LONG_UUID_LENGTH            16

//needed for device Informaiton Service
#define BLE_PACKET_SIZE              200                //BLE Packet length -header
#define Device_Local_Name           "Heimdall"          // Device Local Name
#define MANUFACTURER_NAME           "Alphatronics"      // Manufacturer Name
#define MODEL_NUM                   "0.1.1"
#define HARDWARE_REVISION           "0.1.1"
#define FIRMWARE_REVISION           "0.1.1"
#define SOFTWARE_REVISION           "0.1.1"

//enter the base UUID of UART Service. For example, Here is the Nordic Uart UUID
const uint8_t  ServiceBaseUUID[LONG_UUID_LENGTH] = {
    0x6E, 0x40, 0x00, 0x00, 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};
const uint16_t ServiceShortUUID                 = 0x0001;
const uint16_t CharacteristicShortUUID          = 0x0002;
const uint8_t  ServiceUUID[LONG_UUID_LENGTH] = {
    0x6E, 0x40, (uint8_t)(ServiceShortUUID >> 8), (uint8_t)(ServiceShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};

const uint8_t  CharacteristicUUID[LONG_UUID_LENGTH] = {
    0x6E, 0x40, (uint8_t)(CharacteristicShortUUID >> 8), (uint8_t)(CharacteristicShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};

#if BLE_PACKET_SIZE == 0
#error "Fill the configurations with the details like UUID, Name, BLE_PACKET_Size etc!"
#endif
#endif
