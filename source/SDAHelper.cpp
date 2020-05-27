/*
 * ----------------------------------------------------------------------------
 * Copyright 2020 ARM Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
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
 * ----------------------------------------------------------------------------
 */

#include "SDAHelper.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "factory_configurator_client.h"
#include "ftcd_comm_base.h"
#include "key_config_manager.h"
#include "mbed-trace-helper.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed_stats_helper.h"
#include "mcc_common_setup.h"
#include "pal.h"
#include "sda_demo.h"
#include "sda_macros.h"
#include "sda_status.h"
#include "secure_device_access.h"

extern const uint8_t MBED_CLOUD_TRUST_ANCHOR_PK[];
extern const uint32_t MBED_CLOUD_TRUST_ANCHOR_PK_SIZE;
extern const char MBED_CLOUD_TRUST_ANCHOR_PK_NAME[];
size_t param_size = 0;

char *get_endpoint_name() {
	kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
	char *endpoint_name = NULL;
	size_t endpoint_buffer_size;
	size_t endpoint_name_size;
	// Get endpoint name data size
	kcm_status =
		kcm_item_get_data_size((const uint8_t *)g_fcc_endpoint_parameter_name,
							   strlen(g_fcc_endpoint_parameter_name),
							   KCM_CONFIG_ITEM, &endpoint_name_size);
	if (kcm_status != KCM_STATUS_SUCCESS) {
		tr_error("kcm_item_get_data_size failed (%u)", kcm_status);
		return NULL;
	}

	endpoint_buffer_size = endpoint_name_size + 1; /* for '\0' */

	endpoint_name = (char *)malloc(endpoint_buffer_size);
	memset(endpoint_name, 0, endpoint_buffer_size);
	kcm_status = kcm_item_get_data(
		(const uint8_t *)g_fcc_endpoint_parameter_name,
		strlen(g_fcc_endpoint_parameter_name), KCM_CONFIG_ITEM,
		(uint8_t *)endpoint_name, endpoint_name_size, &endpoint_name_size);
	if (kcm_status != KCM_STATUS_SUCCESS) {
		free(endpoint_name);
		tr_error("kcm_item_get_data failed (%u)", kcm_status);
		return NULL;
	}

	return endpoint_name;
}

bool process_request_fetch_response(const uint8_t *request,
									uint32_t request_size, uint8_t *response,
									size_t response_max_size,
									size_t *response_actual_size) {
	sda_status_e sda_status = SDA_STATUS_SUCCESS;
	// Call to sda_operation_process to process current message, the response
	// message will be returned as output.
	sda_status = sda_operation_process(request, request_size,
									   *application_callback, NULL, response,
									   response_max_size, response_actual_size);
	if (sda_status != SDA_STATUS_SUCCESS) {
		tr_error("Secure-Device-Access operation process failed (%u)",
				 sda_status);
	}

	if (*response_actual_size != 0) {
		return true;
	} else {
		return false;
	}
}

sda_status_e application_callback(sda_operation_ctx_h handle,
								  void *callback_param) {
	sda_status_e sda_status = SDA_STATUS_SUCCESS;
	sda_status_e sda_status_for_response = SDA_STATUS_SUCCESS;
	sda_command_type_e command_type = SDA_OPERATION_NONE;
	const uint8_t *func_callback_name;
	size_t func_callback_name_size;
	bool success = false;  // assume error

	SDA_UNUSED_PARAM(callback_param);
	char response[ResponseBufferLength] = {};
	sda_status = sda_command_type_get(handle, &command_type);
	if (sda_status != SDA_STATUS_SUCCESS) {
		tr_error("Secure-Device-Access failed getting command type (%u)",
				 sda_status);
		sda_status_for_response = sda_status;
		goto out;
	}

	// Currently only SDA_OPERATION_FUNC_CALL is supported
	if (command_type != SDA_OPERATION_FUNC_CALL) {
		tr_error("Got invalid command-type (%u)", command_type);
		sda_status_for_response = SDA_STATUS_INVALID_REQUEST;
		goto out;
	}

	func_callback_name = NULL;
	func_callback_name_size = 0;

	sda_status = sda_func_call_name_get(handle, &func_callback_name,
										&func_callback_name_size);
	if (sda_status != SDA_STATUS_SUCCESS) {
		tr_error(
			"Secure-Device-Access failed getting function callback name (%u)",
			sda_status);
		sda_status_for_response = sda_status;
		goto out;
	}

	tr_info("Function callback is %.*s", (int)func_callback_name_size,
			func_callback_name);

	// Check permission
	sda_status = is_operation_permitted(handle, func_callback_name,
										func_callback_name_size);
	if (sda_status != SDA_STATUS_SUCCESS) {
		tr_error("%.*s operation not permitted (%u)",
				 (int)func_callback_name_size, func_callback_name, sda_status);
		sda_status_for_response = sda_status;
		goto out;
	}

	/***
	* The following commands represents two demos as listed below:
	*   - MWC (Mobile World Congress):
	*     1. "configure"
	*     2. "read-data"
	*     3. "update"
	*
	*   - Hannover Mess:
	*     1. "diagnostics"
	*     2. "restart"
	*     3. "update"
	*
	*   - Note that "update" is a common command for both.
	*/
	/***
	* configure is the task to write the content coming in the SDA Request in
	* the file.
	* Function param that comes with "configure" request contains the data that
	* needed to write
	* as well as the path where the file will be written.
	*/
	if (memcmp(func_callback_name, "configure", func_callback_name_size) == 0) {
		const uint8_t **param_data = (const uint8_t **)calloc(
			500, sizeof(uint8_t *));	//Buffer size to hold the data params coming from sda.
		if (!param_data) {
			tr_error("Can not allocate memory for param_data, not enough space");
			sda_status_for_response = SDA_STATUS_ERROR;
			goto out;
		}
		sda_status = sda_func_call_data_parameter_get(handle, 0, param_data,
													  &param_size);
		if (sda_status != SDA_STATUS_SUCCESS) {
			tr_error("sda_func_call_data_parameter_get() (%u)", sda_status);
			sda_status_for_response = sda_status;
			free(param_data);
			param_data = NULL;
			goto out;
		}
		char *fetch_data = (char *)malloc(param_size * sizeof(uint8_t));
		if (!fetch_data) {
			free(param_data);
			tr_error(
				"Can not allocate memory for fetch data, not enough space");
			sda_status_for_response = SDA_STATUS_ERROR;
			goto out;
		}
		memset(fetch_data, 0, param_size);
		memcpy(&(fetch_data[0]), &(param_data[0][0]), param_size);
		success = demo_callback_writedata((uint8_t *)fetch_data, param_size,
										  response);
		free(param_data);
		free(fetch_data);
		if (!success) {
			tr_error("demo_callback_writedata() failed");
			sda_status_for_response = SDA_STATUS_OPERATION_EXECUTION_ERROR;
			goto out;
		}
	}
	/***
	 * This function reads the data that is there in the file.
	 * There is only one param with this function and that is path of the file
	 * that user want to read.
	 * This function is extracting the function param from the SDA request and
	 * call the demo function to get the response.
	*/
	else if (memcmp(func_callback_name, "read-data", func_callback_name_size) == 0) {
		const uint8_t **param_data = (const uint8_t **)calloc(500, sizeof(uint8_t *));
		if (!param_data) {
			tr_error("Can not allocate memory for param_data, not enough space");
			sda_status_for_response = SDA_STATUS_ERROR;
			goto out;
		}
		sda_status = sda_func_call_data_parameter_get(handle, 0, param_data,
													  &param_size);
		if (sda_status != SDA_STATUS_SUCCESS) {
			tr_error("Please give the file path (%u)", sda_status);
			sda_status_for_response = sda_status;
			free(param_data);
			goto out;
		}
		char *path = (char *)malloc(param_size * sizeof(char));
		if (!path) {
			free(param_data);
			tr_error("Can not allocate memory for path, not enough space");
			sda_status_for_response = SDA_STATUS_ERROR;
			goto out;
		}
		memset(path, 0, param_size);
		memcpy(&(path[0]), &(param_data[0][0]), param_size);
		success =
			demo_callback_read_data((uint8_t *)path, param_size, response);
		free(param_data);
		free(path);
		if (!success) {
			tr_error("demo_callback_read_data() failed");
			sda_status_for_response = SDA_STATUS_OPERATION_EXECUTION_ERROR;
			goto out;
		}
	} else if (memcmp(func_callback_name, "update", func_callback_name_size) ==
			   0) {
		/***
		* Shows progress indicator on the LCD with percentages, and after a few
		* seconds displays
		* "Firmware update successful" with a success LED.
		* This function has no inbound parameters.
		*/

		// Dispatch function callback
		demo_callback_update();

	} else if (memcmp(func_callback_name, "diagnostics",
					  func_callback_name_size) == 0) {
		/***
		* This function accesses the TEMPERATURE peripheral and query current
		* outside temperature,
		* and displays it on LCD if the provided scope is in form of
		* demo_callback_read_temperature.
		* This function has no inbound parameters.
		*/

		// Dispatch function callback
		success = demo_callback_diagnostics();
		if (!success) {
			tr_error("demo_callback_diagnostics() failed");
			sda_status_for_response = SDA_STATUS_OPERATION_EXECUTION_ERROR;
			goto out;
		}

	} else if (memcmp(func_callback_name, "restart", func_callback_name_size) ==
			   0) {
		/***
		* This function accesses the LCD peripheral and sets the current outside
		* temperature.
		* The provided scope must be in form of
		* demo_callback_update_temperature.
		* It gets the temperature value to set as a parameter
		*/

		// Dispatch function callback
		demo_callback_restart();

	} else {
		tr_error("Unsupported callback function name (%.*s)",
				 (int)func_callback_name_size, func_callback_name);
		sda_status_for_response = SDA_STATUS_INVALID_REQUEST;
		goto out;
	}

	sda_status =
		sda_response_data_set(handle, (uint8_t *)response, strlen(response));
	if (sda_status != SDA_STATUS_SUCCESS) {
		tr_error("sda_response_data_set failed (%u)", sda_status);
		sda_status_for_response = sda_status;
		goto out;
	}

// flow succeeded
out:

	if ((sda_status_for_response != SDA_STATUS_SUCCESS) &&
		(sda_status_for_response != SDA_STATUS_NO_MORE_SCOPES)) {
		// Notify some fault happen (only if not 'access denied')
		display_faulty_message("Bad Request");
	}

	return sda_status_for_response;
}

sda_status_e is_operation_permitted(sda_operation_ctx_h operation_context,
									const uint8_t *func_name,
									size_t func_name_size) {
	sda_status_e status;
	const uint8_t *scope;
	size_t scope_size;
	scope = NULL;
	scope_size = 0;

	// Get next available scope in list
	status = sda_scope_get_next(operation_context, &scope, &scope_size);

	// Check if end of scope list reached
	if (status == SDA_STATUS_NO_MORE_SCOPES) {
		tr_error("No match found for operation, permission denied");
		goto access_denied;
	}

	if (status != SDA_STATUS_SUCCESS) {
		tr_error("Failed getting scope, permission denied");
		goto access_denied;
	}

	if ((scope == NULL) || (scope_size == 0)) {
		tr_warn("Got empty or invalid scope, skipping this scope");
		return SDA_STATUS_NO_MORE_SCOPES;
	}

	// Check operation is in scope

	// Check that function name has the exact scope size
	if (scope_size != func_name_size) {
	}

	// Check that function name and scope are binary equal
	if (memcmp(func_name, scope, func_name_size) != 0) {
	}

	tr_info("Operation in scope, access granted");

	return SDA_STATUS_SUCCESS;  // operation permitted

access_denied:

	// Access denied

	tr_error("Operation not in scope, access denied");

	// display_faulty_message("Access Denied");

	return status;
}

bool factory_setup(void) {
#if MBED_CONF_APP_DEVELOPER_MODE == 1
	kcm_status_e kcm_status = KCM_STATUS_SUCCESS;
#endif
	fcc_status_e fcc_status = FCC_STATUS_SUCCESS;
	bool status = true;

	// In both of this cases we call fcc_verify_device_configured_4mbed_cloud()
	// to check if all data was provisioned correctly.

	// Initializes FCC to be able to call FCC APIs
	// TBD: SDA should be able to run without FCC init
	fcc_status = fcc_init();
	if (fcc_status != FCC_STATUS_SUCCESS) {
		status = false;
		tr_error("Failed to initialize Factory-Configurator-Client (%u)",
				 fcc_status);
		goto out;
	}

#if MBED_CONF_APP_DEVELOPER_MODE == 1
	// Storage delete
	fcc_status = fcc_storage_delete();
	if (fcc_status != FCC_STATUS_SUCCESS) {
		tr_error("Storage format failed (%u)", fcc_status);
		status = false;
		goto out;
	}
	// Call developer flow
	tr_cmdline("Start developer flow");
	fcc_status = fcc_developer_flow();
	if (fcc_status != FCC_STATUS_SUCCESS) {
		tr_error("fcc_developer_flow failed (%u)", fcc_status);
		status = false;
		goto out;
	}

	// Store trust anchor
	// Note: Until TA will be part of the developer flow.
	tr_info("Store trust anchor");
	kcm_status = kcm_item_store(
		(const uint8_t *)MBED_CLOUD_TRUST_ANCHOR_PK_NAME,
		strlen(MBED_CLOUD_TRUST_ANCHOR_PK_NAME), KCM_PUBLIC_KEY_ITEM, true,
		MBED_CLOUD_TRUST_ANCHOR_PK, MBED_CLOUD_TRUST_ANCHOR_PK_SIZE, NULL);
	if (kcm_status != KCM_STATUS_SUCCESS) {
		tr_error("kcm_item_store failed (%u)", kcm_status);
		status = false;
		goto out;
	}
#endif

	fcc_status = fcc_verify_device_configured_4mbed_cloud();
	if (fcc_status != FCC_STATUS_SUCCESS) {
		status = false;
		goto out;
	}

// Get endpoint name
// status = get_endpoint_name();
// if (status != true) {
//     tr_error("get_endpoint_name failed");
// }
out:
	// Finalize FFC
	fcc_status = fcc_finalize();
	if (status == false) {
		return false;
	} else {
		if (fcc_status != FCC_STATUS_SUCCESS) {
			tr_error("Failed finalizing Factory-Configurator-Client");
			return false;
		}
	}
	return status;
}
