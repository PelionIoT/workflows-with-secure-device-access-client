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

// Note: this macro is needed on armcc to get the the PRI*32 macros
// from inttypes.h in a C++ code.
#ifndef __SDAHELPER_H__
#define __SDAHELPER_H__

#include "mcc_common_setup.h"
#include "sda_macros.h"
#include "pal.h"
#include "factory_configurator_client.h"
#include "key_config_manager.h"
#include "factory_configurator_client.h"
#include "mcc_common_setup.h"
#include "sda_macros.h"
#include "pal.h"
#include "key_config_manager.h"
#include "sda_status.h"
#include "secure_device_access.h"
#include "ftcd_comm_base.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "mbed_stats_helper.h"
#include "sda_demo.h"

#define ResponseBufferLength	2000
#define PathLength				60

bool factory_setup(void);
sda_status_e is_operation_permitted(sda_operation_ctx_h operation_context, const uint8_t *func_name, size_t func_name_size);
sda_status_e application_callback(sda_operation_ctx_h handle, void *callback_param);
bool process_request_fetch_response(const uint8_t *request,uint32_t request_size,uint8_t *response,size_t response_max_size, size_t *response_actual_size);
char* get_endpoint_name();
#endif
