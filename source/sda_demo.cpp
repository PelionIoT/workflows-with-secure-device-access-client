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

#ifndef SDAE_INTERNAL_DEMO

#include "include/sda_demo.h"
#include "mbed.h"
#include "PinNames.h"
#include "mbed-trace/mbed_trace.h"
#include "mbed-trace-helper.h"
#include "mcc_common_setup.h"
#include "LittleFileSystem.h"
#include "BlockDevice.h"

/////////////////////// DEFINITIONS ///////////////////////
#define TRACE_GROUP           "sdae"

// Color value is simple bitfield of 3 led bits (blue, green, red)
#define LED_CL_BLACK  7 // OFF
#define LED_CL_RED    6
#define LED_CL_GREEN  5
#define LED_CL_YELLOW 4
#define LED_CL_BLUE   3
#define LED_CL_PINK   2
#define LED_CL_CYAN   1
#define LED_CL_WHITE  0

#define LED_OFF 1
#define LED_ON  0

#define LED_RED LED1
#define LED_GREEN LED2
#define LED_BLUE LED3

/////////////////////// STRUCTURES ////////////////////////


///////////////////////// GLOBALS /////////////////////////

DigitalOut led_red(LED_RED, LED_OFF);
DigitalOut led_green(LED_GREEN, LED_OFF);
DigitalOut led_blue(LED_BLUE, LED_OFF);

//////////////////////////////////////////////////////////


static void set_led_color(uint32_t color)
{
    // extract bits from bitfield
    led_red = (color & 1);
    led_green = (color & 2) >> 1;
    led_blue = (color & 4) >> 2;
}

static uint32_t get_led_color()
{
    // combine bits and return bitfield
    return (uint32_t)(led_red.read() | (led_green.read() << 1) | (led_blue.read() << 2));
}

static void emulate_operation(const char* operation_name, uint32_t color, size_t duration_in_sec)
{
    uint32_t prev_color = get_led_color();

    if (operation_name) {
        tr_cmdline("proccess operation %s", operation_name);
    }

    for (size_t i = 0; i < duration_in_sec * 2; i++)
    {
        set_led_color(LED_CL_BLACK);
        mcc_platform_do_wait(250);
        set_led_color(color);
        mcc_platform_do_wait(250);
    }

    set_led_color(prev_color);
}

void demo_setup(void)
{
    set_led_color(LED_CL_GREEN);
}

void display_faulty_message(const char *fault_message)
{
    tr_error("error message: %s", fault_message);
    emulate_operation(NULL, LED_CL_RED, 10);
}

// path+content
bool demo_callback_writedata(uint8_t* data) {
    char* token;
    token = strtok((char*)data,"^");

    char *final_path = (char*)calloc((strlen(token)+4),sizeof(char));
    sprintf(final_path,"/fs/%s",(char*)token);
    printf("final-path:%s\n",final_path)
    tr_info("Writing file\n");
    FILE* f = fopen(final_path, "w");
    if(f!=NULL) {
        token=strtok(NULL, "^");
        printf("%s",token);
        fprintf(f,"%s\r\n", (const char*)token);
        fflush(f);
        fclose(f);
        free(final_path);
        //memset(final_path, 0, strlen((const char*)token)+4);
        token = NULL;
        return true;
    }
    else{
        free(final_path);
        return false;
    }
}
// response pointer should be allocated bedore calling this function
bool demo_callback_read_data(uint8_t* path, char* response)
{
    if((path == NULL) || (*path == ' ')){
        tr_error("Please provide the path of the file");
        return false;
    }
    uint8_t path_len = strlen((const char*)path)+4;
    char final_path[path_len];
    // if(final_path==NULL){
    //     tr_error("Can not allocate memory of final_path");
    //     return false;
    // }
    sprintf(final_path,"/fs/%s",(char*)path);
    printf("Final path:%s",final_path);
    FILE* r = fopen(final_path, "r");
    if(!r){
        printf("Can not open file");
        return false;
        }
    printf("File read starting");
    fscanf(r, "%[^\n]", response);
    fflush(r);
    fclose(r);
    tr_info("File read complete");
    //free(final_path);
    //final_path = NULL;
    memset(final_path, 0, strlen((const char*)path)+4);
    return true;
    // //emulate_operation("read", LED_CL_BLUE, 10);

    // BlockDevice *bd = BlockDevice::get_default_instance();
    // LittleFileSystem fs("fs");
    // int err = fs.mount(bd);
    // printf("%s\n", (err ? "Fail :(" : "OK"));
    // if (err) {
    //     // Reformat if we can't mount the filesystem
    //     // this should only happen on the first boot
    //     printf("No filesystem found, formatting... ");
    //     fflush(stdout);
    //     err = fs.reformat(bd);
    //     printf("%s\n", (err ? "Fail :(" : "OK"));
    //     if (err) {
    //         error("error: %s (%d)\n", strerror(-err), err);
    //     }
    // }
    // printf("Writing file");
    // FILE* f = fopen("/fs/pb.txt", "w+");
    // if(f==NULL) {
    //     return false;
    // }
    // fprintf(f,"Hello Prakhar \r\n");
    // fflush(f);
    // fclose(f);
    // FILE* r = fopen("/fs/pb.txt", "r");
    // if(r==NULL)return false;
    // tr_error("File read starting");
    // int c;
    // while((c = fgetc(r)) != EOF) {
    //     putchar(c);
    // }
    // fflush(r);
    // fclose(r);
    // tr_error("File read complete");
}

bool demo_callback_configure(int64_t temperature)
{
    emulate_operation("configure", LED_CL_WHITE, 5);
    tr_error("Outside temperature: 28");
    return true;
}

bool demo_callback_update(void)
{
    //emulate_operation("read", LED_CL_CYAN, 10);
    return true;
}

bool demo_callback_restart(void)
{
    emulate_operation("read", LED_CL_YELLOW, 10);
    return true;
}

bool demo_callback_diagnostics(void)
{
    emulate_operation("read", LED_CL_PINK, 5);
    return true;
}

#endif // SDAE_INTERNAL_DEMO
