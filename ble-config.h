
#ifndef __BLE_CONFIG_UUID__
#define __BLE_CONFIG_UUID__

#include <stdint.h>

#define LONG_UUID_LENGTH            16

//Define BLE packet Length that will transmit in each transition, should be '8' Less than MTU.

#define BLE_PACKET_SIZE

/***
For Device Informaiton Service in BLE
 * Device Local Name to advertise in BLE
 * Manufacturer Name
 * Model Number
 * Hardware Revision
 * Firmware Revision
 * Software Revision
*/
#define Device_Local_Name
#define MANUFACTURER_NAME
#define MODEL_NUM
#define HARDWARE_REVISION
#define FIRMWARE_REVISION
#define SOFTWARE_REVISION

//enter the base UUID of the Service needed to brodcast the BLE.
/**
 * Enter the Base UUID of the service you are going to advertise in array.
 * Enter the Short UUID of the Service.
 * Enter the Short Characteristic UUID.
 * Enter the Service UUID
 * Enter the characteristic UUID.
 */
const uint8_t  ServiceBaseUUID[LONG_UUID_LENGTH]    = {};
const uint16_t ServiceShortUUID                     = ;
const uint16_t CharacteristicShortUUID              = ;
const uint8_t  ServiceUUID[LONG_UUID_LENGTH]        = {};
const uint8_t  CharacteristicUUID[LONG_UUID_LENGTH] = {};

#if BLE_PACKET_SIZE
    #error "Fill the configurations with the details like UUID, Name, BLE_PACKET_Size etc!"
#endif
#endif