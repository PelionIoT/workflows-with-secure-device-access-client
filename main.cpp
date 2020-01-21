/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include "mbed_cloud_client_user_config.h"
#include <stdint.h>
#include <stdio.h>
#include "events/EventQueue.h"
#include "platform/Callback.h"
#include "platform/NonCopyable.h"
#include "BLEProcess.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GapAdvertisingParams.h"
#include "ble/GapAdvertisingData.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "mbed_stats_helper.h"
#include "string"
#include "comm_interface.h"
#include "sdahelper.h"


using mbed::callback;

#define TRACE_GROUP           "main"

static int g_demo_main_status = EXIT_FAILURE;   // holds the demo main task return code

bool success;

static void demo_main(){
	mcc_platform_sw_build_info();

    tr_cmdline("Secure-Device-Access initialization");

    // Initialize storage
    success = mcc_platform_storage_init() == 0;
    if (success != true) {
        tr_error("Failed initializing mcc platform storage\n");
        return;
    }

    // Avoid standard output buffering
    setvbuf(stdout, (char *)NULL, _IONBF, 0);

    success = factory_setup();
    if (success != true) {
        tr_error("Demo setup failed");
        return;
    }
    char* endpoint = get_endpoint_name();
    printf("Endpoint: %s", endpoint);
	Comm_interface* comm_interface = new Comm_interface();
	comm_interface->init(endpoint);
	comm_interface->start();
}

int main(void) {
	success = mbed_trace_helper_init(TRACE_ACTIVE_LEVEL_ALL | TRACE_MODE_COLOR, false);
    if (!success) {
        return EXIT_FAILURE;
    }
	 success = (mcc_platform_init() == 0);
    if (success) {
        success = mcc_platform_run_program(&demo_main);
    }
    return success ? g_demo_main_status : EXIT_FAILURE;
}