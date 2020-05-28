# Changelog

## Release v1.0.0 - 2020-05-27

### Features
- Supported target [ST DISCO-L475VG-IOT01A](https://os.mbed.com/platforms/ST-Discovery-L475E-IOT01A/).
- Integrated littlefs to mange the files on the target device.
    - Supports filename of length 256.
    - Default mount point is `/fs`.
- Supported workflow task types `configure` and `read-data`.
- Works with v1.0.1 of PDM mobile app.
- Using Mbed OS version 5.15.
- Using Device Management Client version 4.2.1.
- Provide BLE communication interface with PDM mobile application.
- Provide options to configure BLE discover service uuid and service characteristics uuid. 
- Provide option to configure BLE device local name, manufacture name, model number,  hardware revision, firmware revision and software revision.
- Developed and implemented a simple low-overhead fragmentation  protocol to handle packets bigger than the BLE MTU size.
- Read and write max file size is 1kB.
- Execute workflows which are authenticated by SDA client part of PDMC.