# Secure-Device-Access-Client-Over-BLE

This is an example to demonstrate the working of SDA(Secure Device Access client) over BLE. The board we used for this example is DISCO-L475VG-IOT01A which has the Bluetooth Low Energy support.

## Dependencies

There are few dependencies to build this project:

* Mbed-CLI - https://github.com/ARMmbed/mbed-cli
* GCC_ARM Toolchain - https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm

For the Ubuntu system, to load the binary, you will need:

* PyOCD - https://github.com/mbedmicro/pyOCD
* GNU Debugger for ARM Cortex processors.

## Build Steps

* Download and Navigate to this repository.
* Open up the terminal and run `mbed deploy`. this will download all the necessary files to build this project. These include MbedOS, Mbed-Cloud-Client.
* To run this code into the developer mode, you will need the security certificate from the portal.Just download and replace `mbed_cloud_dev_credentials.c` from the portal. To do that, steps are written after this segment.
* You will also need a trust anchor, in the portal, go to Device Identity-> Trust anchor and copy the trust anchor from the portal in PEM Format and run `python create_trust_anchor_dev_cred.py -t "your trust anchor"`
* Now, you will have to add some configurations for your BLE, Open up the ```ble-config.h``` in root folder of the project and add the configurations like device name, service UUID etc. An example is given at the end of this segment.
* After this, you are ready to compile. Just run
`mbed compile -t GCC_ARM -m DISCO-L475VG-IOT01A`
* Flash the generated binary to your board.

## Developer Mode

You need to add the mbed_cloud_dev_credentials.c file to the config folder. You need a user account in Device Management Cloud to be able to generate a developer certificate. To obtain the developer certificate, follow these steps:

Go to Device identity -> Security.
Click actions and Generate developer certificate,
Give a name and an optional description to the certificate.
Download the certificate file mbed_cloud_dev_credentials.c

## Factory Provisioning

For factory provisioning, refer this doc: https://www.pelion.com/docs/device-management-provision/1.2/introduction/index.html

## Example for BLE Config File

Please note that this is just an example, you may want to use your own UUID.

* Add BLE Packet Size between 23 to 244. It should be '8' less than the MTU. For example:

```C
#define BLE_PACKET_SIZE             200
#define Device_Local_Name           "DISCO"
#define MANUFACTURER_NAME           "ARM"
#define MODEL_NUM                   "0.1.1"
#define HARDWARE_REVISION           "0.1.1"
#define FIRMWARE_REVISION           "0.1.1"
#define SOFTWARE_REVISION           "0.1.1"

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
```

### Side Note:
To change the trace level of logs,

* Open the mbed_app.json present in the root folder of the project, in the option: ` mbed-trace-max-level ` change the value of ` TRACE_LEVEL_ERROR ` to `TRACE_LEVEL_INFO` and compile the project.

* Just connect the device with the USB Cable, open up the Putty or any other terminal and configure the baud-rate to 115200 and open the port.
