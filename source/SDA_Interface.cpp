// ----------------------------------------------------------------------------
// Copyright 2017-2019 ARM Ltd.
//  
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//  
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "sda_interface.h"
#include "include/sda_demo.h"
#include "include/sdahelper.h"

#define TRACE_GROUP           "sdaI"


int SDAInterface::sdaProcess(uint8_t* buffer, uint8_t buffSize ){
    sda_init();
    sda_status_e sda_status = SDA_STATUS_SUCCESS;
    //char* request = NULL;
    uint32_t request_size = 485;
    uint8_t response[SDA_RESPONSE_HEADER_SIZE + sizeof("This is app data buffer")];
    size_t response_max_size = sizeof(response);
    size_t response_actual_size = 0;
    printf("Buffer - %s Size - %d \r\n", buffer, buffSize);
     //request = (char*)calloc(46, sizeof(char));
      if (pal_osGetTime() != 0) {
        // Note - please keep after sda_init() to assure clock is disabled.
        // This is workaround for special case where MCCE is run before SDAE.
        // In such case, the device time will be set upon connection to LWM2M.
        // If the device has no RTC (real time clock), the time on the device won't be correct after reboot
        // And it will lead to failure in validation of access token time.
        tr_warn("For demo propose only, setting time to 0");
        pal_osSetTime(0);
    }
    uint8_t request[485] = {130, 3, 89,1, 224, 210, 132, 67, 161, 1, 38, 160, 89, 1, 148, 131, 27, 227, 112, 189, 255, 182, 146, 168, 85, 131, 1, 105, 99, 111,
    110, 102, 105, 103, 117,114, 101, 129, 101, 104, 101, 108, 108, 111, 89, 1, 116, 216, 61, 210,132, 67, 161, 1, 38, 160, 89, 1, 38, 169, 12, 120, 26, 114,
    101, 97, 100, 45, 100, 97, 116, 97, 32, 99, 111, 110, 102, 105, 103 ,117 ,114, 101, 32 ,117, 112, 100, 97, 116, 101, 24, 25, 161, 1, 164, 1, 2, 32, 1, 33,
    88, 32, 136, 22, 235, 120, 187, 220, 214, 6, 53, 211, 254, 166, 251, 242, 121, 194, 13 ,71, 100, 224, 151, 3 ,128, 151, 244, 76 ,155, 221, 139, 98, 3, 142,
    34, 88, 32, 241, 37, 253, 184, 167, 134, 42, 158, 108, 129, 168, 33, 95, 117, 236, 226, 87, 130, 148, 180, 147, 22, 249, 62, 99, 239, 230, 114, 82, 86, 183,
    147, 2, 120, 32, 48, 49, 54, 97, 101, 52, 57, 97, 102, 98, 54, 51, 57, 50, 52, 54, 55, 99, 101, 99, 50, 56, 53, 52, 48, 48, 48, 48, 48, 48, 48, 48, 3, 129, 
    120, 35, 101 ,112, 58, 48, 49, 54 ,101, 101, 97, 100, 50, 57, 51, 101, 98, 57, 50, 54, 99, 97, 53, 55, 98, 97, 57, 50, 55, 48, 51, 99, 48, 48, 48, 48, 48, 4, 
    26, 94, 33, 106, 185, 5, 26, 94, 32, 25, 57 ,6 ,26, 94, 32, 25, 57, 1, 120, 72 ,109, 98, 101, 100, 46, 116, 97, 46, 49 ,54 ,68 ,51 ,52 ,48 ,49, 49, 48, 67, 53, 
    53, 56 ,66 ,56 ,65 ,65 ,66 ,70 ,65 ,65 ,52 ,55 ,68 ,66 ,48 ,48 ,52 ,67 ,66 ,52 ,50 ,57 ,56 ,48 ,66 ,56 ,51 ,68 ,55 ,49 ,66 ,54 ,49 ,69 ,69 ,56 ,52 ,67 ,50 ,68 ,
    68 ,67 ,70 ,70 ,57 ,68 ,69 ,48 ,70 ,55 ,56 ,55 ,69 ,7 ,80 ,1 ,111 ,173 ,98 ,136 ,198 ,154 ,152 ,58 ,118 ,1 ,189 ,0 ,0 ,0 ,0 ,88 ,64 ,49 ,95 ,181 ,233 ,62 ,60 ,
    143 ,115 ,133 ,243 ,100 ,198 ,40 ,239 ,144 ,179 ,190 ,128 ,58 ,128 ,70 ,109 ,219 ,248 ,168 ,151 ,18 ,237 ,196 ,118 ,10 ,46 ,24 ,182 ,92 ,91 ,201 ,191 ,82 ,25, 
    91 ,10 ,4 ,164 ,113 ,175 ,231 ,98 ,11 ,153 ,225 ,2, 57 ,217 ,3 ,218 ,174 ,172 ,203 ,108 ,166 ,31, 215 ,139 ,88 ,64 ,137 ,245 ,191 ,6 ,170 ,2 ,208 ,78 ,237 ,48 ,
    140 ,55 ,125 ,227 ,77 ,223 ,158 ,106 ,206, 149 ,52 ,22, 142 ,94 ,14 ,164, 8 ,69, 97, 119 ,94 ,234 ,200 ,137 ,158 ,57 ,232 ,159 ,243 ,206 ,255 ,61 ,100 ,14, 180 ,
    160 ,210 ,73 ,105 ,25 ,137 ,133 ,203 ,184 ,47 ,211 ,39 ,246 ,240 ,124 ,152 ,165 ,81 ,31};
    bool success = process_request_fetch_response((uint8_t*)request, request_size, response, response_max_size, &response_actual_size);
        if (!success) {
            printf("Failed processing request message\r\n");
        }

    //process_request_fetch_response((uint8_t*)request, request_size, response, response_max_size, &response_actual_size);
}

