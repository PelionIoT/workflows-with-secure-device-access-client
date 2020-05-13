#!/bin/sh
patch mbed-os/features/FEATURE_BLE/targets/TARGET_CORDIO/driver/CordioHCIDriver.cpp < patch/Patchfile1
patch mbed-os/features/FEATURE_BLE/targets/TARGET_CORDIO/stack_adaptation/hci_tr.c < patch/Patchfile2