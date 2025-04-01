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


typedef struct {
    bool current;
    bool last;
    uint32_t stable_count;
} DebouncedButton;

DebouncedButton p1_ul, p1_ur, p1_cn, p1_dl, p1_dr;
DebouncedButton p2_ul, p2_ur, p2_cn, p2_dl, p2_dr;
DebouncedButton test, service, mode_switch;


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

bool change_mode_switch = false;

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

const uint8_t pin_list[13] = {21, 6, 10, 19, 8, 27, 0, 2, 17, 4, 14, 15, 24};


struct inputArray input_mux[MUX_COUNT];


// note that lights array are ACTIVE HIGH
//this is probably necessary as it's used to send PIUIO packets
struct lightsArray lights = {
    .data = {0x00}
};


//TODO: this sucks
void debounce_button(DebouncedButton *button, bool new_state) {
    if (new_state != button->current) {
        button->stable_count++;
        if (button->stable_count >= DEBOUNCE_COUNT) {
            button->current = new_state;
            button->stable_count = 0;
        }
    } else {
        button->stable_count = 0;
    }
}

//TODO: this also sucks
void update_input_mux() {

    if (!DEBOUNCING) {
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
    change_mode_switch = gpio_get(24);
    }

    if (DEBOUNCING) {
    bool raw_p1_ul = gpio_get(19);
    bool raw_p1_ur = gpio_get(6);
    bool raw_p1_cn = gpio_get(10);
    bool raw_p1_dl = gpio_get(21);
    bool raw_p1_dr = gpio_get(8);

    bool raw_p2_ul = gpio_get(27);
    bool raw_p2_ur = gpio_get(0);
    bool raw_p2_cn = gpio_get(2);
    bool raw_p2_dl = gpio_get(17);
    bool raw_p2_dr = gpio_get(4);

    bool raw_test = gpio_get(14);
    bool raw_service = gpio_get(15);
    bool raw_mode_switch = gpio_get(24); //this should be the usr button on knockoff rp2040s

    debounce_button(&p1_ul, raw_p1_ul);
    debounce_button(&p1_ur, raw_p1_ur);
    debounce_button(&p1_cn, raw_p1_cn);
    debounce_button(&p1_dl, raw_p1_dl);
    debounce_button(&p1_dr, raw_p1_dr);

    debounce_button(&p2_ul, raw_p2_ul);
    debounce_button(&p2_ur, raw_p2_ur);
    debounce_button(&p2_cn, raw_p2_cn);
    debounce_button(&p2_dl, raw_p2_dl);
    debounce_button(&p2_dr, raw_p2_dr);

    debounce_button(&test, raw_test);
    debounce_button(&service, raw_service);
    debounce_button(&mode_switch, raw_mode_switch);

    input.p1_ul = p1_ul.current;
    input.p1_ur = p1_ur.current;
    input.p1_cn = p1_cn.current;
    input.p1_dl = p1_dl.current;
    input.p1_dr = p1_dr.current;

    input.p2_ul = p2_ul.current;
    input.p2_ur = p2_ur.current;
    input.p2_cn = p2_cn.current;
    input.p2_dl = p2_dl.current;
    input.p2_dr = p2_dr.current;

    input.p1_coin = true;
    input.p2_coin = true;

    input.test = test.current;
    input.service = service.current;
    input.clear = true;

    change_mode_switch = mode_switch.current;
    }

}

void input_task() {
    uint32_t current_ts = board_millis();

    update_input_mux();

    
    if (config_mode) {
        if ((!input.test && last_input.test) || (!input.test && last_input.test)) {
            input_mode_tmp = (input_mode_tmp + 1) % INPUT_MODE_COUNT;
        }
    }

    //TODO: no clue why it doesn't work, too tired to fix it
/*
    if (change_mode_switch) {
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

//this mode is still kind of shotty, it works but due to it's speed bouncing remains an issue in 1 sensor per panel setups

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

void gpio_def() {
    int i;
    for (i = 0; i < 13; i++) {
        gpio_init(pin_list[i]);
        gpio_set_dir(pin_list[i], false);
        gpio_pull_up(pin_list[i]);
    }
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