/**********************************************************/
/*  SPDX-License-Identifier: MIT                          */
/*  https://github.com/sugoku/piuio-pico-brokeIO          */
/**********************************************************/

#include "bsp/board.h"
#include "device/usbd.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "hardware/watchdog.h"

#include "piuio_structs.h"
#include "piuio_config.h"
#include "input_mode.h"

#include "input_mux4067.h"
#include "lights_latch32.h"

#include "reports/hid_report.h"
#include "reports/keyboard_report.h"
#include "reports/lxio_report.h"
#include "reports/switch_report.h"
#include "reports/xinput_report.h"

#ifdef ENABLE_WS2812_SUPPORT
#include "piuio_ws2812.h"
#endif

// const uint8_t pos[] = { 3, 0, 2, 1, 4 }; // don't touch this
// i touched it

int input_mode = -1;

bool select_mode = false;

// set pad lights based on whether an arrow is pressed or not, bypassing the host
// automatically enabled for all modes besides PIUIO and LXIO
bool direct_lights = false;

// have brokeIO automatically switch mux between reads instead of waiting for
// host signal (required for everything except PIUIO)
bool auto_mux = false;

// merge all sensors for a panel, so that the host receives all sensor readings
// ORed together (stepping on at least one sensor always triggers the arrow)
bool merge_mux = false;

// used for auto mux mode
uint8_t current_mux = 0;

uint32_t last_service_ts;
uint8_t hid_rx_buf[32];

#define XINPUT_OUT_SIZE 32
uint8_t xinput_out_buffer[XINPUT_OUT_SIZE] = {};

// PIUIO input and output data
// note that inputs are ACTIVE LOW
struct inputArray input = {
    .data = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

struct inputArray last_input = {
    .data = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

struct inputArray input_mux[5];

// note that lights array are ACTIVE HIGH
struct lightsArray lights = {
    .data = {0x00}
};

extern uint32_t mux4067_vals[5];

void update_input_mux() {
    uint32_t buf_mux_global;
    uint32_t buf_mux_p1;
    uint32_t buf_mux_p2;
    
    if (merge_mux || select_mode) {
        uint32_t merged = mux4067_merged(mux4067_vals);
        buf_mux_global = merged;
        buf_mux_p1 = merged;
        buf_mux_p2 = merged;
    } else {
        buf_mux_global = mux4067_vals[MUX_GLOBAL];
        buf_mux_p1 = mux4067_vals[lights.p1_mux];
        buf_mux_p2 = mux4067_vals[lights.p2_mux];
    }
    
    // logic is negative
    input.p1_ul = !GETBIT(buf_mux_p1, MUX4067_P1_UPLEFT);
    input.p1_ur = !GETBIT(buf_mux_p1, MUX4067_P1_UPRIGHT);
    input.p1_cn = !GETBIT(buf_mux_p1, MUX4067_P1_CENTER);
    input.p1_dl = !GETBIT(buf_mux_p1, MUX4067_P1_DOWNLEFT);
    input.p1_dr = !GETBIT(buf_mux_p1, MUX4067_P1_DOWNRIGHT);

    input.p2_ul = !GETBIT(buf_mux_p2, MUX4067_P2_UPLEFT);
    input.p2_ur = !GETBIT(buf_mux_p2, MUX4067_P2_UPRIGHT);
    input.p2_cn = !GETBIT(buf_mux_p2, MUX4067_P2_CENTER);
    input.p2_dl = !GETBIT(buf_mux_p2, MUX4067_P2_DOWNLEFT);
    input.p2_dr = !GETBIT(buf_mux_p2, MUX4067_P2_DOWNRIGHT);

    input.p1_coin = !GETBIT(buf_mux_global, MUX4067_P1_COIN);
    input.p2_coin = !GETBIT(buf_mux_global, MUX4067_P2_COIN);

    input.test = !GETBIT(buf_mux_global, MUX4067_TEST);
    input.service = !GETBIT(buf_mux_global, MUX4067_SERVICE);
    input.clear = !GETBIT(buf_mux_global, MUX4067_CLEAR);

    for (int i = 0; i < 5; i++) {
        // logic is negative
        input_mux[i].p1_ul = !GETBIT(mux4067_vals[i], MUX4067_P1_UPLEFT);
        input_mux[i].p1_ur = !GETBIT(mux4067_vals[i], MUX4067_P1_UPRIGHT);
        input_mux[i].p1_cn = !GETBIT(mux4067_vals[i], MUX4067_P1_CENTER);
        input_mux[i].p1_dl = !GETBIT(mux4067_vals[i], MUX4067_P1_DOWNLEFT);
        input_mux[i].p1_dr = !GETBIT(mux4067_vals[i], MUX4067_P1_DOWNRIGHT);

        input_mux[i].p2_ul = !GETBIT(mux4067_vals[i], MUX4067_P2_UPLEFT);
        input_mux[i].p2_ur = !GETBIT(mux4067_vals[i], MUX4067_P2_UPRIGHT);
        input_mux[i].p2_cn = !GETBIT(mux4067_vals[i], MUX4067_P2_CENTER);
        input_mux[i].p2_dl = !GETBIT(mux4067_vals[i], MUX4067_P2_DOWNLEFT);
        input_mux[i].p2_dr = !GETBIT(mux4067_vals[i], MUX4067_P2_DOWNRIGHT);

        input_mux[i].p1_coin = !GETBIT(mux4067_vals[i], MUX4067_P1_COIN);
        input_mux[i].p2_coin = !GETBIT(mux4067_vals[i], MUX4067_P2_COIN);

        input_mux[i].test = !GETBIT(mux4067_vals[i], MUX4067_TEST);
        input_mux[i].service = !GETBIT(mux4067_vals[i], MUX4067_SERVICE);
        input_mux[i].clear = !GETBIT(mux4067_vals[i], MUX4067_CLEAR);
    }
}

void input_task() {
    mux4067_update(lights.p1_mux, lights.p2_mux);

    update_input_mux();

    if (select_mode) {
        if ((!input.p1_dl && last_input.p1_dl) || (!input.p2_dl && last_input.p2_dl)) {
            if (input_mode > 0)
                input_mode--;
        } else if ((!input.p1_dr && last_input.p1_dr) || (!input.p2_dr && last_input.p2_dr)) {
            if (input_mode < INPUT_MODE_COUNT - 1)
                input_mode++;
        } else if ((!input.test && last_input.test) || (!input.test && last_input.test)) {
            input_mode = (input_mode + 1) % INPUT_MODE_COUNT;
        }
    }

    // keep track of last time service button was held down
    if (last_input.service && !input.service) {
        last_service_ts = board_millis();
    }
    // if button held down long enough, enter or exit mode select
    if (!input.service && board_millis() - last_service_ts > SETTINGS_THRESHOLD) {
        if (!select_mode) {
            select_mode = true;
        } else {
            select_mode = false;
            // save changes for mode to flash memory and reset device!
            write_input_mode(input_mode);
            // enable watchdog and enter infinite loop to reset
            watchdog_enable(1, 1);
            while(1);
        }
    }

    last_input = input;
}

void jamma_led_update(uint32_t* buf) {
    uint32_t time = board_millis();
    uint8_t blink_count = (time / SERVICE_BLINK_INTERVAL) % (input_mode + 1);
    // blink for BLINK_LENGTH every BLINK_INTERVAL ms for input_mode times.
    // then leave an empty spot at the end to separate the next blinking cycle
    bool state = (blink_count <= input_mode) && (time % SERVICE_BLINK_INTERVAL <= SERVICE_BLINK_LENGTH);

    SETORCLRBIT(*buf, LATCH_JAMMA_LED, state);
}

void lights_task() {
    #ifdef ENABLE_WS2812_SUPPORT
    ws2812_lock_mtx();
    #endif

    uint32_t buf = 0;

    if (select_mode) {
        // force direct_lights behavior
        uint32_t in_buf = mux4067_merged(mux4067_vals);

        SETORCLRBIT(buf, LATCH_P1L_UPLEFT, input_mode & 0b100);
        SETORCLRBIT(buf, LATCH_P1L_CENTER, input_mode & 0b10);
        SETORCLRBIT(buf, LATCH_P1L_UPRIGHT, input_mode & 0b1);

        SETORCLRBIT(buf, LATCH_P1L_DOWNLEFT, GETBIT(in_buf, MUX4067_P1_DOWNLEFT));
        SETORCLRBIT(buf, LATCH_P1L_DOWNRIGHT, GETBIT(in_buf, MUX4067_P1_DOWNRIGHT));

        SETORCLRBIT(buf, LATCH_P2L_UPLEFT, input_mode & 0b100);
        SETORCLRBIT(buf, LATCH_P2L_CENTER, input_mode & 0b10);
        SETORCLRBIT(buf, LATCH_P2L_UPRIGHT, input_mode & 0b1);

        SETORCLRBIT(buf, LATCH_P2L_DOWNLEFT, GETBIT(in_buf, MUX4067_P2_DOWNLEFT));
        SETORCLRBIT(buf, LATCH_P2L_DOWNRIGHT, GETBIT(in_buf, MUX4067_P2_DOWNRIGHT));

        jamma_led_update(&buf);
        
    } else if (direct_lights) {  // technically it could be direct_lights && !merge_mux
        uint32_t in_buf = mux4067_merged(mux4067_vals);

        SETORCLRBIT(buf, LATCH_P1L_UPLEFT, GETBIT(in_buf, MUX4067_P1_UPLEFT));
        SETORCLRBIT(buf, LATCH_P1L_UPRIGHT, GETBIT(in_buf, MUX4067_P1_UPRIGHT));
        SETORCLRBIT(buf, LATCH_P1L_CENTER, GETBIT(in_buf, MUX4067_P1_CENTER));
        SETORCLRBIT(buf, LATCH_P1L_DOWNLEFT, GETBIT(in_buf, MUX4067_P1_DOWNLEFT));
        SETORCLRBIT(buf, LATCH_P1L_DOWNRIGHT, GETBIT(in_buf, MUX4067_P1_DOWNRIGHT));

        SETORCLRBIT(buf, LATCH_P2L_UPLEFT, GETBIT(in_buf, MUX4067_P2_UPLEFT));
        SETORCLRBIT(buf, LATCH_P2L_UPRIGHT, GETBIT(in_buf, MUX4067_P2_UPRIGHT));
        SETORCLRBIT(buf, LATCH_P2L_CENTER, GETBIT(in_buf, MUX4067_P2_CENTER));
        SETORCLRBIT(buf, LATCH_P2L_DOWNLEFT, GETBIT(in_buf, MUX4067_P2_DOWNLEFT));
        SETORCLRBIT(buf, LATCH_P2L_DOWNRIGHT, GETBIT(in_buf, MUX4067_P2_DOWNRIGHT));

        SETORCLRBIT(buf, LATCH_P1_S0, lights.p1_mux & 0b1);
        SETORCLRBIT(buf, LATCH_P1_S1, lights.p1_mux & 0b10);
        SETORCLRBIT(buf, LATCH_P2_S0, lights.p2_mux & 0b1);
        SETORCLRBIT(buf, LATCH_P2_S1, lights.p2_mux & 0b10);
        
        // CLRBIT(buf, LATCH_CABL_MARQ1, lights.l1_halo);
        // CLRBIT(buf, LATCH_CABL_MARQ2, lights.l2_halo);
        // CLRBIT(buf, LATCH_CABL_MARQ3, lights.r1_halo);
        // CLRBIT(buf, LATCH_CABL_MARQ4, lights.r2_halo);
        // CLRBIT(buf, LATCH_CABL_NEON, lights.bass_light);

        SETBIT(buf, LATCH_ALWAYS_ON);
        // CLRBIT(buf, LATCH_COIN_COUNTER);
        SETBIT(buf, LATCH_JAMMA_LED);
    } else {
        SETORCLRBIT(buf, LATCH_P1L_UPLEFT, lights.p1_ul_light);
        SETORCLRBIT(buf, LATCH_P1L_UPRIGHT, lights.p1_ur_light);
        SETORCLRBIT(buf, LATCH_P1L_CENTER, lights.p1_cn_light);
        SETORCLRBIT(buf, LATCH_P1L_DOWNLEFT, lights.p1_dl_light);
        SETORCLRBIT(buf, LATCH_P1L_DOWNRIGHT, lights.p1_dr_light);

        SETORCLRBIT(buf, LATCH_P2L_UPLEFT, lights.p2_ul_light);
        SETORCLRBIT(buf, LATCH_P2L_UPRIGHT, lights.p2_ur_light);
        SETORCLRBIT(buf, LATCH_P2L_CENTER, lights.p2_cn_light);
        SETORCLRBIT(buf, LATCH_P2L_DOWNLEFT, lights.p2_dl_light);
        SETORCLRBIT(buf, LATCH_P2L_DOWNRIGHT, lights.p2_dr_light);

        SETORCLRBIT(buf, LATCH_P1_S0, lights.p1_mux & 0b1);
        SETORCLRBIT(buf, LATCH_P1_S1, lights.p1_mux & 0b10);
        SETORCLRBIT(buf, LATCH_P2_S0, lights.p2_mux & 0b1);
        SETORCLRBIT(buf, LATCH_P2_S1, lights.p2_mux & 0b10);

        SETORCLRBIT(buf, LATCH_CABL_MARQ1, lights.l1_halo);
        SETORCLRBIT(buf, LATCH_CABL_MARQ2, lights.l2_halo);
        SETORCLRBIT(buf, LATCH_CABL_MARQ3, lights.r1_halo);
        SETORCLRBIT(buf, LATCH_CABL_MARQ4, lights.r2_halo);
        SETORCLRBIT(buf, LATCH_CABL_NEON, lights.bass_light);

        SETBIT(buf, LATCH_ALWAYS_ON);
        // CLRBIT(buf, LATCH_COIN_COUNTER);
        SETBIT(buf, LATCH_JAMMA_LED);
    }

    lights_send(&buf);

    #ifdef ENABLE_WS2812_SUPPORT
    ws2812_unlock_mtx();
    #endif

    if (auto_mux) {
        current_mux = (current_mux + 1) % 4;
        lights.p1_mux = current_mux;
        lights.p2_mux = current_mux;
    }
}

// void receive_xinput_report(void)
// {
// 	if (
// 		tud_ready() &&
// 		(endpoint_out != 0) && (!usbd_edpt_busy(0, endpoint_out)))
// 	{
// 		usbd_edpt_claim(0, endpoint_out);									 // Take control of OUT endpoint
// 		usbd_edpt_xfer(0, endpoint_out, xinput_out_buffer, XINPUT_OUT_SIZE); // Retrieve report buffer
// 		usbd_edpt_release(0, endpoint_out);									 // Release control of OUT endpoint
// 	}
// }

void receive_report(uint8_t *buffer) {
	// if (input_mode == INPUT_MODE_XINPUT) {
	// 	receive_xinput_report();
	// 	memcpy(buffer, xinput_out_buffer, XINPUT_OUT_SIZE);
	// }
}

// bool send_xinput_report(void *report, uint8_t report_size)
// {
// 	bool sent = false;

// 	if (
// 		tud_ready() &&											// Is the device ready?
// 		(endpoint_in != 0) && (!usbd_edpt_busy(0, endpoint_in)) // Is the IN endpoint available?
// 	)
// 	{
// 		usbd_edpt_claim(0, endpoint_in);								// Take control of IN endpoint
// 		usbd_edpt_xfer(0, endpoint_in, (uint8_t *)report, report_size); // Send report buffer
// 		usbd_edpt_release(0, endpoint_in);								// Release control of IN endpoint
// 		sent = true;
// 	}

// 	return sent;
// }

void send_report(void *report, uint16_t report_size) {
	static uint8_t previous_report[CFG_TUD_ENDPOINT0_SIZE] = { };

	if (tud_suspended())
		tud_remote_wakeup();

	if (memcmp(previous_report, report, report_size) != 0)
	{
		bool sent = false;
		switch (input_mode)
		{
			case INPUT_MODE_XINPUT:
				//sent = send_xinput_report(report, report_size);
				break;

			default:
				//sent = send_hid_report(0, report, report_size);
				break;
		}

		if (sent)
			memcpy(previous_report, report, report_size);
	}
}

// returns size, sets pointer to report pointer
uint16_t get_report(void* report) {
    switch (input_mode)
	{
		case INPUT_MODE_GAMEPAD:
			return hid_get_report(report, &input);

        case INPUT_MODE_LXIO:
			return lxio_get_report(report, &input, input_mux);

        case INPUT_MODE_KEYBOARD:
			return keyboard_get_report(report, &input);

		case INPUT_MODE_XINPUT:
			return xinput_get_report(report, &input);

		case INPUT_MODE_SWITCH:
			return switch_get_report(report, &input);

		default:
			return hid_get_report(report, &input);
	}
}

void hid_task() {
    if (input_mode == INPUT_MODE_PIUIO)
        return;

    // USB FEATURES : Send/Get USB Features (including Player LEDs on X-Input)
    void* report;
    uint16_t size = get_report(report);
    send_report(report, size);
    receive_report(hid_rx_buf);
}

void init() {
    get_input_mode();

    switch (input_mode)
	{
        case INPUT_MODE_PIUIO:
            direct_lights = false;
            auto_mux = false;
            merge_mux = false;
            break;

		case INPUT_MODE_GAMEPAD:
			direct_lights = false;
            auto_mux = true;
            merge_mux = true;
            break;

        case INPUT_MODE_LXIO:
			direct_lights = false;
            auto_mux = true;
            merge_mux = false;
            break;

        case INPUT_MODE_KEYBOARD:
			direct_lights = true;
            auto_mux = true;
            merge_mux = true;
            break;

		case INPUT_MODE_XINPUT:
			direct_lights = true;
            auto_mux = true;
            merge_mux = true;
            break;

		case INPUT_MODE_SWITCH:
			direct_lights = true;
            auto_mux = true;
            merge_mux = true;
            break;
	}
}

int main() {
    board_init();

    // Init WS2812B
    #ifdef ENABLE_WS2812_SUPPORT
    ws2812_init(&lights);
    #endif

    mux4067_init();
    lights_init();

    init();

    tusb_init();

    // Main loop
    while (true) {
        tud_task(); // tinyusb device task

        lights_task();  // update mux before reading in input
        input_task();

        hid_task();
    }

    return 0;
}


// ------------ tinyusb callbacks ------------

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) {
    // nothing to with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP) return true;

    if (input_mode == INPUT_MODE_PIUIO) {
        // Request 0xAE = IO Time
        if (request->bRequest == 0xAE) {
            switch (request->bmRequestType) {
                case 0x40:
                    return tud_control_xfer(rhport, request, (void *)&lights.data, 8);
                case 0xC0:
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

    if (select_mode) return 0;

	uint16_t report_size = get_report(buffer);

	return report_size;
}

// set_report needed here for HID lights and LXIO
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void) instance;

    if (!select_mode)
        if (input_mode == INPUT_MODE_LXIO)
            if (report_type == HID_REPORT_TYPE_OUTPUT)  // note that no report ID is specified in the LXIO's device descriptor
                lxio_set_report(buffer, bufsize, &lights);
}