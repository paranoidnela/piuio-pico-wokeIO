/*******************************************************************************************/
/*  SPDX-License-Identifier: MIT                                                           */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 48productions, therathatter, dj505, sugoku  */
/*  https://github.com/sugoku/piuio-pico-brokeIO                                           */
/*******************************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "bsp/board.h"
#include "device/usbd.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"

#include "piuio_structs.h"
#include "piuio_config.h"
#include "input_mode.h"

#include "reports/hid_report.h"
#include "reports/keyboard_report.h"
#include "reports/lxio_report.h"
#include "reports/switch_report.h"
#include "reports/xinput_report.h"
#include "reports/gamecube_report.h"

#include "xinput_driver.h"
#include "device/usbd_pvt.h"

int input_mode = -1;
int input_mode_tmp = -1;

bool config_mode = false;
bool config_switched = false;

//all of this is just false all the time because we have no jamma connector
// is JAMMA_Q toggled on or off
bool q = false;
bool q_toggle = false;

bool last_q = false;

bool jamma_w = false;
bool jamma_x = false;
bool jamma_y = false;
bool jamma_z = false;

bool mode_switch = false;

// used for auto mux mode
uint8_t current_mux = 0;

uint32_t last_service_ts;
uint32_t last_p1_cn_ts;
uint32_t last_p2_cn_ts;
uint8_t hid_rx_buf[32];

extern uint8_t xinput_out_buffer[XINPUT_OUT_SIZE];

// PIUIO input and output data
// note that inputs are ACTIVE LOW
struct inputArray input = {
    .data = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

struct inputArray last_input = {
    .data = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

// use this array for PIUIO when all inputs should be off
const uint8_t all_inputs_off[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct inputArray input_mux[MUX_COUNT];


// note that lights array are ACTIVE HIGH
//this is probably necessary as it's used to send PIUIO packets
struct lightsArray lights = {
    .data = {0x00}
};


void update_input_mux() {
    input.p1_ul = gpio_get(19);
    input.p1_ur = gpio_get(6);
    input.p1_cn = gpio_get(10);
    input.p1_dl = gpio_get(21);
    input.p1_dr = gpio_get(8);

    input.p2_ul = gpio_get(27);
    input.p2_ur = gpio_get(0);
    input.p2_cn = gpio_get(2);
    input.p2_dl = gpio_get(17);
    input.p2_dr = gpio_get(4);

    input.p1_coin = true;
    input.p2_coin = true;

    input.test = gpio_get(14);
    input.service = gpio_get(15);
    input.clear = true;

    //TODO: this sucks and is only needed for lxio, I think
    for (int i = 0; i < MUX_COUNT; i++) {
        // logic is negative
        input_mux[i].p1_ul = input.p1_ul;
        input_mux[i].p1_ur = input.p1_ur;
        input_mux[i].p1_cn = input.p1_cn;
        input_mux[i].p1_dl = input.p1_dl;
        input_mux[i].p1_dr = input.p1_dr;

        input_mux[i].p2_ul = input.p2_ul;
        input_mux[i].p2_ur = input.p2_ur;
        input_mux[i].p2_cn = input.p2_cn;
        input_mux[i].p2_dl = input.p2_dl;
        input_mux[i].p2_dr = input.p2_dr;

        input_mux[i].p1_coin = input.p1_coin;
        input_mux[i].p2_coin = input.p2_coin;

        input_mux[i].test = input.test;
        input_mux[i].service = input.service;
        input_mux[i].clear = input.clear;
    }

    //this is to change modes without any of the holding buttons bs
    mode_switch = gpio_get(22); 

}

void input_task() {
    uint32_t current_ts = board_millis();

    //TODO: implement gpio debounce
    update_input_mux();

    
    if (config_mode) {
        if ((!input.test && last_input.test) || (!input.test && last_input.test)) {
            input_mode_tmp = (input_mode_tmp + 1) % INPUT_MODE_COUNT;
        }
    }
    
    //TODO: no clue why it doesn't work, too tired to fix it
/*
    if (mode_switch) {
        input_mode_tmp = input_mode + 1;
        write_input_mode(input_mode_tmp);
        watchdog_enable(1, 1);
        while(1);
    }
*/

    // keep track of last time service button was held down
    // also don't do this if test is also pressed down
    if (last_input.service && !input.service && input.test) {
        last_service_ts = current_ts;
        config_switched = true;
    }
    // if button held down long enough, enter or exit config mode
    if (config_switched && !input.service && current_ts - last_service_ts > SETTINGS_THRESHOLD && input.test) {
        if (!config_mode) {
            input_mode_tmp = input_mode;
            config_mode = true;
        } else {
            config_mode = false;
            // save changes for mode to flash memory and reset device!
            write_input_mode(input_mode_tmp);
            // enable watchdog and enter infinite loop to reset
            watchdog_enable(1, 1);
            while(1);
        }
        config_switched = false;
        // use config_switched to make sure that when we enter config_mode
        // we don't instantly exit it (requiring another button press to switch config_mode)
    }

    last_input = input;
    last_q = q;
}

void receive_report(uint8_t *buffer) {
    if (input_mode == INPUT_MODE_XINPUT) {
        receive_xinput_report();
        memcpy(buffer, xinput_out_buffer, XINPUT_OUT_SIZE);
    }
}

void send_report(void *report, uint16_t report_size) {
    static uint8_t previous_report[CFG_TUD_ENDPOINT0_SIZE] = { };

    if (report_size == 0 || report == NULL)
        return;

    if (tud_suspended())
        tud_remote_wakeup();

    if (memcmp(previous_report, report, report_size) != 0) {
        bool sent = false;
        switch (input_mode) {
            case INPUT_MODE_XINPUT:
                sent = send_xinput_report(report, report_size);
                break;

            default:
                if (tud_hid_ready())
                    sent = tud_hid_report(0, report, report_size);
                break;
        }

        if (sent)
            memcpy(previous_report, report, report_size);
    }
}

// returns size, sets pointer to report pointer
// we need double pointers to ensure that we change the address of void*
// and that this change persists outside this function
uint16_t get_report(void** report) {
    switch (input_mode) {
        case INPUT_MODE_GAMEPAD:
            return hid_get_report((HIDReport**)report, &input);

        case INPUT_MODE_LXIO: 
            return lxio_get_report((uint8_t**)report, &input, input_mux);

        case INPUT_MODE_KEYBOARD:
            return keyboard_get_report((KeyboardReport**)report, &input);

        case INPUT_MODE_XINPUT:
            return xinput_get_report((XInputReport**)report, &input);

        case INPUT_MODE_SWITCH:
            return switch_get_report((SwitchReport**)report, &input, q_toggle, jamma_w, jamma_x, jamma_y, jamma_z);

        case INPUT_MODE_GAMECUBE:
            return gamecube_get_report((GameCubeReport**)report, &input, &last_input, q_toggle, jamma_w, jamma_x, jamma_y, jamma_z);

        default:
            return 0;
    }
}

void hid_task() {
    if (config_mode || input_mode == INPUT_MODE_PIUIO)
        return;


    // USB FEATURES : Send/Get USB Features (including Player LEDs on X-Input)
    void* report = NULL;
    uint16_t size = get_report(&report);
    send_report(report, size);
    receive_report(hid_rx_buf);
}

//TODO: this majorly sucks
void gpio_def() {
    gpio_init(21);
    gpio_init(6);
    gpio_init(10);
    gpio_init(19);
    gpio_init(8);
    gpio_init(27);
    gpio_init(0);
    gpio_init(2);
    gpio_init(17);
    gpio_init(4);
    gpio_init(14);
    gpio_init(15);
    gpio_init(22);

    gpio_set_dir(21, false);
    gpio_set_dir(6, false);
    gpio_set_dir(10, false);
    gpio_set_dir(19, false);
    gpio_set_dir(8, false);
    gpio_set_dir(27, false);
    gpio_set_dir(0, false);
    gpio_set_dir(2, false);
    gpio_set_dir(17, false);
    gpio_set_dir(4, false);
    gpio_set_dir(14, false);
    gpio_set_dir(15, false);
    gpio_set_dir(22, false);

    gpio_pull_up(21);
    gpio_pull_up(6);
    gpio_pull_up(10);
    gpio_pull_up(19);
    gpio_pull_up(8);
    gpio_pull_up(27);
    gpio_pull_up(0);
    gpio_pull_up(2);
    gpio_pull_up(17);
    gpio_pull_up(4);
    gpio_pull_up(14);
    gpio_pull_up(15);
    gpio_pull_up(22);
}

int main() {
    board_init();
    gpio_def();
    tusb_init();

    // Main loop
    while (true) {

        tud_task(); // tinyusb device task
        input_task();
        hid_task();
    }

    return 0;
}


// ------------ tinyusb callbacks ------------

const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count)
{
    // only switch the driver when we are using xinput; otherwise, use defaults and do nothing

    if (input_mode == INPUT_MODE_XINPUT) {
        *driver_count = 1;
        return &xinput_driver;
    }
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) {
    // nothing to with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP) return true;

    if (input_mode == INPUT_MODE_PIUIO) {

        #ifdef BENCHMARK
        static uint8_t loop_toggle_out = 0x00;
        static uint8_t loop_toggle_in = 0x00;
        #endif

        // Request 0xAE = IO Time
        if (request->bRequest == 0xAE) {
            switch (request->bmRequestType) {
                case 0x40:
                    if (config_mode)
                        return false;
                    return tud_control_xfer(rhport, request, (void *)&lights.data, 8);
                case 0xC0:
                    // if in config mode, make sure that we turn all inputs off or inputs will get stuck!
                    if (config_mode)
                        return tud_control_xfer(rhport, request, (void *)all_inputs_off, 8);
                    
                    return tud_control_xfer(rhport, request, (void *)&input.data, 8);
                default:
                    return false;
            }
        }
    }

    return false;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
    // TODO: Handle the correct report type, if required
    (void)itf;

    if (config_mode || input_mode == INPUT_MODE_PIUIO) return 0;

    void* report = NULL;
    uint16_t size = get_report(&report);

    if (size == 0 || report == NULL)
        return 0;
        
    memcpy(buffer, report, size);

    return size;
}

// set_report needed here for HID lights and LXIO
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;

    if (!config_mode) {
        if (input_mode == INPUT_MODE_LXIO) {
            if (report_type == HID_REPORT_TYPE_OUTPUT) {
            // do not consider the report type at all! tinyusb ignores it and sets it to HID_REPORT_TYPE_INVALID (0)
            // also note that no report ID is specified in the LXIO's device descriptor
            lxio_set_report(buffer, bufsize, &lights);
            }
        } 
        
        else if (input_mode == INPUT_MODE_GAMECUBE) {
            // rumble
        }
    }
}