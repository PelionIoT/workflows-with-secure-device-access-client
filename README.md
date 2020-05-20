# workflows-with-secure-device-access-client

`workflows-with-secure-device-access-client` is an Mbed OS application that communicates with Personal Digital Assistant (PDA) and executes Secure Device Access (SDA) authorized and authenticated workflows. For information about SDA, please see the [public SDA documentation](https://www.pelion.com/docs/device-management/current/device-management/secure-device-access.html).

The application implements a Bluetooth Low Energy (BLE) communication interface with PDA and integrates [littlefs](https://github.com/ARMmbed/littlefs) to read and write files to and from the target device.

This document explains how to build `workflows-with-secure-device-access-client` and flash the binary onto your device:

- [Prerequisites](#prerequisites)
- [Building the application](#building-the-application)
- [Flashing the binary](#flashing-the-binary)
- [Debugging](#debugging)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Hardware requirements
- An [ST DISCO-L475VG-IOT01A](https://os.mbed.com/platforms/ST-Discovery-L475E-IOT01A/) board.
- A USB cable (USB Type A to Micro-AB).

### Working environment requirements
- A Device Management account with the secure device access feature and workflow management service activated.
- Operating systems we have tested: Ubuntu 18.04.

### Library dependencies
- mbed-os v5.15.1
- mbed-cloud-client v4.2.1

Mbed CLI installs these dependencies when you run the `mbed deploy` command as part of the [build steps](#build-steps).

### Tools and toolchains
- [Install Python 2.7](https://www.python.org/download/releases/2.7/).
- [Install Mbed CLI](https://os.mbed.com/docs/mbed-os/v5.15/quick-start/offline-with-mbed-cli.html).
- [GNU Arm GCC Compiler](https://www.pelion.com/docs/device-management/current/cloud-requirements/tool-requirements.html#notes-for-toolchains).
- [pyOCD](https://pypi.org/project/pyocd/).

After you install Mbed CLI and GNU Arm GCC Compiler, set the path to the Arm embedded toolchain:
```
mbed config -G GCC_ARM_PATH "~/<gcc-arm-none-eabi-6-xxx>/bin"
```

## Building the application

1. Clone the application and navigate to the folder:

    ```
    git clone https://github.com/ARMmbed/workflows-with-secure-device-access-client
    cd workflows-with-secure-device-access-client
    ```

1. Provision device credentials:

    - If you are using [developer credentials](https://www.pelion.com/docs/device-management/current/sda/obtaining-a-bootstrap-certificate-and-trust-anchor.html#in-developer-mode):
        1. Overwrite the `mbed_cloud_dev_credentials.c` template file in the `workflows-with-secure-device-access-client` folder with the `mbed_cloud_dev_credentials.c` file that you [download from the Portal](https://www.pelion.com/docs/device-management/current/sda/obtaining-a-bootstrap-certificate-and-trust-anchor.html#generate-a-bootstrap-certificate).
        1. From the `workflows-with-secure-device-access-client` folder, overwrite the `mbed_cloud_trust_anchor_credentials.c` template file with your [trust anchor](https://www.pelion.com/docs/device-management/current/sda/obtaining-a-bootstrap-certificate-and-trust-anchor.html#obtain-a-trust-anchor):
            ```
            python create_trust_anchor_dev_cred.py -t "<trust anchor>"
            ```
            For example:
            ```
            python create_trust_anchor_dev_cred.py -t "-----BEGIN PUBLIC KEY-----MFkaEqYHKoZIzj0CAQYIKoZIzj0GAQcDHgAEbiRnZgdzoBpySFDPVPFp3J7yOmrOXJ09O5qVUMOD5knUjX7YbQBF0ueJWPz6tkTGbzORAwDzvRXYUA7vZpB+og==-----END PUBLIC KEY-----"
            ```
            Note: Be sure to remove any line breaks (\n or \r) in the trust anchor before running `create_trust_anchor_dev_cred.py`.

    - In production mode, provision and store your credentials on your device, as described in the [provisioning documentation](https://www.pelion.com/docs/device-management/current/provisioning-process/index.html).

1. Install library dependencies:
    ```
    mbed deploy
    ```
    This downloads all libraries required to build the application.

1. Navigate to the `mbed-os` folder in `workflows-with-secure-device-access-client` and apply Mbed OS patches:
    ```
    git am ../patches/*
    ```

1. Configure the BLE interface by setting values for the parameters defined in `ble-config.h`.

    Note: `BLE_PACKET_SIZE` should be 12 to 232 bytes. We recommend you use a large packet size to reduce the number of BLE packets, increase throughput and reduce power consumption.

    Example:

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

1. Compile the application:
    ```
    mbed compile -t GCC_ARM -m DISCO_L475VG_IOT01A
    ```

## Flashing the binary

1. Connect the board to your computer using USB.
1. In the terminal, run:
    ```
    pyocd gdbserver
    ```
    Press Enter while holding the Reset button on the board.

1. In another terminal, navigate to the `BUILD` folder in `workflows-with-secure-device-access-client`, and run:
    ```
    arm-none-eabi-gdb
    ```
    This launches the GDB (GNU Debugger) terminal.

1. To connect to the pyOCD server, inside the GDB terminal, run:
    ```
    target remote:3333
    ```

1. Load the binary onto your board:
    ```
    load workflows-with-secure-device-access-client.hex
    ```
    The LED blinks rapidly. Wait for it to finish.

## Debugging

1. Connect the board to your computer using USB.
1. Use `mbedls` to detect and list Mbed Enabled devices connected to the computer.
1. Use PuTTY or any other serial console with 115200 baud 8-N-1 configuration to view the logs.

Note: To change the trace level of logs, open the `mbed_app.json` file in the root folder of the project. Under ` mbed-trace-max-level `, change the value of `TRACE_LEVEL_ERROR` to `TRACE_LEVEL_INFO`. Compile the project and flash the new binary.

## Troubleshooting

Please see [GitHub issues](https://github.com/armPelionEdge/workflows-with-secure-device-access-client/issues) for solutions to common build errors.
