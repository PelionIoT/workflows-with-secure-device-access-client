# ble-service-example-internal
This is an example to demonstrate the working of SDA(Secure Device Access client) over BLE. The board we used for this example is DISCO-L475VG-IOT01A which has the Bluetooth Low Energy support.

## Build Steps
* Download and Navigate to this repository.
* Open up the terminal and run `mbed deploy`. this will download all the necessary files to build this project. These include MbedOS, Mbed-Cloud-Client.
* To run this code into the developer mode(the default one), you will need the security certificate from the portal.Download and replace your security credential file from the portal.
* You will also need a trust anchor, copy the trust anchor from the portal and run `python create_trust_anchor_dev_cred.py -t "your trust anchor"`
* Now, you will have to add some configurations for your BLE, Open up the ```ble-config-uuid.h``` in root folder of the project and add the configurations like device name, service UUID etc.
* After this, you are ready to compile. Just run
`mbed compile -t GCC_ARM -m DISCO-L475VG-IOT01A`
* Flash the generated binary to your board.