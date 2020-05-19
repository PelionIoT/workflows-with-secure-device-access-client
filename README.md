# workflows-with-secure-device-access-client

A mbedOS application which communicates with personal digital assistant (PDA) and executes SDA authorized and authenticated workflows. For information about Secure Device Access (SDA), please see the [public SDA documentation](https://www.pelion.com/docs/device-management/current/device-management/secure-device-access.html).

It enables a Bluetooth Low Energy (BLE) communication interface with PDA and integrates [littlefs](https://github.com/ARMmbed/littlefs) for reading and writing the files to and from the target device.

## Prerequisites

### Hardware requirements
- A [ST DISCO-L475VG-IOT01A](https://os.mbed.com/platforms/ST-Discovery-L475E-IOT01A/) board
- A USB cable (USB Type A to Micro-AB)

### Working environment requirements
- A Device Management account with the secure device access feature and workflow management service activated.
- Operating systems we have tested with: Ubuntu 18.04

### Library dependencies
- mbed-os v5.15.1
- mbed-cloud-client v4.2.1

These will be installed when you run `mbed deploy` command as instructed in the Build Steps section of this doc.

### Tools and toolchains
- [Install Mbed CLI](https://os.mbed.com/docs/mbed-os/v5.15/quick-start/offline-with-mbed-cli.html)
- [GNU Arm GCC Compiler](https://www.pelion.com/docs/device-management/current/cloud-requirements/tool-requirements.html#notes-for-toolchains).
- [pyOCD](https://pypi.org/project/pyocd/)

Note: After Mbed CLI is installed, tell Mbed where to find the Arm embedded toolchain.
```
mbed config -G GCC_ARM_PATH "~/<gcc-arm-none-eabi-6-xxx>/bin"
```

## Build Steps
- Clone the application and navigate to the folder.
    ```
    git clone https://github.com/ARMmbed/workflows-with-secure-device-access-client
    cd workflows-with-secure-device-access-client
    ```

- If you are using [developer credentials](https://www.pelion.com/docs/device-management/current/sda/obtaining-a-bootstrap-certificate-and-trust-anchor.html#in-developer-mode), build the target as described below. In production mode, skip these steps, and provision and store your credentials on your target, as described in the [provisioning documentation](https://www.pelion.com/docs/device-management/current/provisioning-process/index.html).
    - Overwrite the mbed_cloud_dev_credentials.c template file in the workflows-with-secure-device-access-client folder with the mbed_cloud_dev_credentials.c file that you [downloaded from the Portal](https://www.pelion.com/docs/device-management/current/sda/obtaining-a-bootstrap-certificate-and-trust-anchor.html#generate-a-bootstrap-certificate).
    - From the workflows-with-secure-device-access-client folder, run the following command to overwrite the mbed_cloud_trust_anchor_credentials.c template file with your [trust anchor](https://www.pelion.com/docs/device-management/current/sda/obtaining-a-bootstrap-certificate-and-trust-anchor.html#obtain-a-trust-anchor):
        ```
        python create_trust_anchor_dev_cred.py -t "<trust anchor>"
        ```
        For example:
        ```
        python create_trust_anchor_dev_cred.py -t "-----BEGIN PUBLIC KEY-----MFkaEqYHKoZIzj0CAQYIKoZIzj0GAQcDHgAEbiRnZgdzoBpySFDPVPFp3J7yOmrOXJ09O5qVUMOD5knUjX7YbQBF0ueJWPz6tkTGbzORAwDzvRXYUA7vZpB+og==-----END PUBLIC KEY-----"
        ```
        Note: Be sure to remove any line breaks (\n or \r) in the trust anchor before running create_trust_anchor_dev_cred.py.

- Install the dependencies.
    ```
    mbed deploy
    ```
    This will download all the necessary libraries required to build this application. Currently, the application is developed with mbed-os v5.15.1 and mbed-cloud-client v4.2.1.

- Apply the patches to mbed-os. Navigate to mbed-os folder in workflows-with-secure-device-access-client.
    ```
    git am ../patches/*
    ```

- Configure the BLE interface by providing the values to the following parameters defined in `ble-config.h`.

    Note: BLE_PACKET_SIZE should range from 12 to 232 bytes. We recommend you to use the large packet size as it will reduce the number of BLE packets thus increasing the throughput and also reducing the power consumption.

    The following values are used as an example -
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

- Compile the application.
    ```
    mbed compile -t GCC_ARM -m DISCO_L475VG_IOT01A
    ```

## Flash the binary

- Connect the target to your computer using USB.
- In the terminal, run
    ```
    pyocd gdbserver
    ```
    Hit enter while holding the reset button on the board.

* In another terminal, navigate to the `BUILD` folder in the workflows-with-secure-device-access-client and run
    ```
    arm-none-eabi-gdb
    ```
    This will launch the gdb terminal.

* Connect to the pyOCD server by running the following command inside the gdb terminal.
    ```
    target remote:3333
    ```

* Load the binary onto your board, run
    ```
    load workflows-with-secure-device-access-client_application.hex
    ```
    The LED blinks rapidly, wait for it to finish.

## Debugging

- Connect the target to your computer using USB.
- Use `mbedls` to detect and list Mbed Enabled devices connected to the computer.
- Use PuTTY or any other serial console with configuration 115200 baud 8-n-1 to view the logs.

Note: To change the trace level of logs, open the `mbed_app.json` present in the root folder of the project, in the option: ` mbed-trace-max-level ` change the value of `TRACE_LEVEL_ERROR` to `TRACE_LEVEL_INFO`. Compile the project and flash the new binary.


## Troubleshooting

Please see the [GitHub issues](https://github.com/armPelionEdge/workflows-with-secure-device-access-client/issues) for solutions to common build errors.
