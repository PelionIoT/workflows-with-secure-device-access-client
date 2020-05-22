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

#include "include/sda_demo.h"
#include "BlockDevice.h"
#include "LittleFileSystem.h"
#include "PinNames.h"
#include "mbed-trace-helper.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed.h"
#include "mcc_common_setup.h"

/////////////////////// DEFINITIONS ///////////////////////
#define TRACE_GROUP "sdad"

// Color value is simple bitfield of 3 led bits (blue, green, red)
#define LED_CL_BLACK 7  // OFF
#define LED_CL_RED 6
#define LED_CL_GREEN 5
#define LED_CL_YELLOW 4
#define LED_CL_BLUE 3
#define LED_CL_PINK 2
#define LED_CL_CYAN 1
#define LED_CL_WHITE 0

#define LED_OFF 1
#define LED_ON 0

#define LED_RED LED1
#define LED_GREEN LED2
#define LED_BLUE LED3

#define SEPERATOR "^"

/////////////////////// STRUCTURES ////////////////////////

///////////////////////// GLOBALS /////////////////////////

DigitalOut led_red(LED_RED, LED_OFF);
DigitalOut led_green(LED_GREEN, LED_OFF);
DigitalOut led_blue(LED_BLUE, LED_OFF);

//////////////////////////////////////////////////////////

static void set_led_color(uint32_t color) {
	// extract bits from bitfield
	led_red = (color & 1);
	led_green = (color & 2) >> 1;
	led_blue = (color & 4) >> 2;
}

static uint32_t get_led_color() {
	// combine bits and return bitfield
	return (uint32_t)(led_red.read() | (led_green.read() << 1) |
					  (led_blue.read() << 2));
}

static void emulate_operation(const char* operation_name, uint32_t color,
							  size_t duration_in_sec) {
	uint32_t prev_color = get_led_color();

	if (operation_name) {
		tr_cmdline("proccess operation %s", operation_name);
	}

	for (size_t i = 0; i < duration_in_sec * 2; i++) {
		set_led_color(LED_CL_BLACK);
		mcc_platform_do_wait(250);
		set_led_color(color);
		mcc_platform_do_wait(250);
	}

	set_led_color(prev_color);
}

void demo_setup(void) { 
	set_led_color(LED_CL_GREEN); 
}

void display_faulty_message(const char* fault_message) {
	tr_error("error message: %s", fault_message);
	emulate_operation(NULL, LED_CL_RED, 10);
}

bool demo_callback_writedata(uint8_t* data, uint16_t data_len, char* response) {
	char* token = strtok((char*)data, SEPERATOR);
	uint8_t path_len = strlen(token) + 4 + 1;
	char final_path[path_len] = {0};
	sprintf(final_path, "/fs/%s", (char*)token);
	tr_info("Writing file in path: %s and data bytes:%d", final_path, data_len);
	FILE* f = fopen(final_path, "w");
	if (f != NULL) {
		token = strtok(NULL, "^");
		tr_info("Data is: %s", token);
		int filebytes = fprintf(f, "%s", token);
		tr_info("File Bytes in buffer:%d",filebytes);
		if (filebytes) {
			sprintf(response, "File write completed!");
			tr_info("File write completed!");
			fflush(f);
			fclose(f);
			return true;
		} else {
			fflush(f);
			fclose(f);
			token = NULL;
			return false;
		}
	} else {
		tr_error("Err: %s", strerror(errno));
		return false;
	}
}
// response pointer should be allocated before calling this function
bool demo_callback_read_data(uint8_t* path, uint8_t path_size, char* response) {
	uint8_t path_len = path_size + 5;
	char final_path[path_len] = {0};
	snprintf(final_path, path_len, "/fs/%s", (char*)path);
	tr_info("Reading data from file-path:%s", final_path);
	FILE* r = fopen(final_path, "r");
	if (!r) {
		tr_error("Can not open file %s", strerror(errno));
		tr_error("Err: %s", strerror(errno));
		return false;
	}
	fseek (r , 0 , SEEK_END);
	int file_size = ftell(r);
	rewind(r);
	tr_info("File read starting");
	int read_bytes = fread (response,1,file_size,r);
	fclose(r);
	tr_info("Data is: %s and bytes read: %d", response, read_bytes);
	tr_info("File read complete");
	return true;
}

bool demo_callback_configure(int64_t temperature) {
	emulate_operation("configure", LED_CL_WHITE, 5);
	tr_error("Outside temperature: 28");
	return true;
}

bool demo_callback_update(void) {
	// emulate_operation("read", LED_CL_CYAN, 10);
	return true;
}

bool demo_callback_restart(void) {
	emulate_operation("read", LED_CL_YELLOW, 10);
	return true;
}

bool demo_callback_diagnostics(void) {
	emulate_operation("read", LED_CL_PINK, 5);
	return true;
}
