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

#include "RS485_protocol.h"

#include "piuio_structs.h"
#include "piuio_config.h"
#include "input_mode.h"

#include "uart_structs.h"

#include "input_mux4067.h"
#include "lights_latch32.h"

#include "reports/hid_report.h"
#include "reports/keyboard_report.h"
#include "reports/lxio_report.h"
#include "reports/switch_report.h"
#include "reports/xinput_report.h"
#include "reports/gamecube_report.h"

// please fix all of this CDC code
enum
{
  ITF_NUM_CDC_0 = 0,
  ITF_NUM_CDC_0_DATA,
  ITF_NUM_TOTAL_SERIAL
};

#include "xinput_driver.h"
#include "device/usbd_pvt.h"

#ifdef ENABLE_WS2812_SUPPORT
#include "piuio_ws2812.h"
#endif

// const uint8_t pos[] = { 3, 0, 2, 1, 4 }; // don't touch this
// i touched it

int input_mode = -1;
int input_mode_tmp = -1;

bool config_mode = false;
bool config_switched = false;

// set pad lights based on whether an arrow is pressed or not, bypassing the host
// automatically enabled for all modes besides PIUIO and LXIO
bool direct_lights = false;

// have brokeIO automatically switch mux between reads instead of waiting for
// host signal (required for everything except PIUIO)
bool auto_mux = false;

// merge all sensors for a panel, so that the host receives all sensor readings
// ORed together (stepping on at least one sensor always triggers the arrow)
bool merge_mux = false;

// this mode is used to confirm the functionality of a brokeIO
// it maps each input in the 4067 mux to its corresponding output on the latch
const bool factory_test_mode = false;

// is JAMMA_Q toggled on or off
bool q_toggle = false;

bool last_q = false;

bool jamma_w = false;
bool jamma_x = false;
bool jamma_y = false;
bool jamma_z = false;

// used for auto mux mode
uint8_t current_mux = 0;

uint32_t serial_lights_buf;

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

struct inputArray input_mux[MUX_COUNT];

// note that lights array are ACTIVE HIGH
struct lightsArray lights = {
    .data = {0x00}
};

extern uint32_t mux4067_vals_db[MUX_COUNT];

void update_input_mux() {
    uint32_t buf_mux_global;
    uint32_t buf_mux_p1;
    uint32_t buf_mux_p2;
    
    if (merge_mux || config_mode) {
        uint32_t merged = mux4067_merged(mux4067_vals_db);
        buf_mux_global = merged;
        buf_mux_p1 = merged;
        buf_mux_p2 = merged;
    } else {
        buf_mux_global = mux4067_vals_db[MUX_GLOBAL];
        buf_mux_p1 = mux4067_vals_db[lights.p1_mux];
        buf_mux_p2 = mux4067_vals_db[lights.p2_mux];
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

    for (int i = 0; i < MUX_COUNT; i++) {
        // logic is negative
        input_mux[i].p1_ul = !GETBIT(mux4067_vals_db[i], MUX4067_P1_UPLEFT);
        input_mux[i].p1_ur = !GETBIT(mux4067_vals_db[i], MUX4067_P1_UPRIGHT);
        input_mux[i].p1_cn = !GETBIT(mux4067_vals_db[i], MUX4067_P1_CENTER);
        input_mux[i].p1_dl = !GETBIT(mux4067_vals_db[i], MUX4067_P1_DOWNLEFT);
        input_mux[i].p1_dr = !GETBIT(mux4067_vals_db[i], MUX4067_P1_DOWNRIGHT);

        input_mux[i].p2_ul = !GETBIT(mux4067_vals_db[i], MUX4067_P2_UPLEFT);
        input_mux[i].p2_ur = !GETBIT(mux4067_vals_db[i], MUX4067_P2_UPRIGHT);
        input_mux[i].p2_cn = !GETBIT(mux4067_vals_db[i], MUX4067_P2_CENTER);
        input_mux[i].p2_dl = !GETBIT(mux4067_vals_db[i], MUX4067_P2_DOWNLEFT);
        input_mux[i].p2_dr = !GETBIT(mux4067_vals_db[i], MUX4067_P2_DOWNRIGHT);

        input_mux[i].p1_coin = !GETBIT(mux4067_vals_db[i], MUX4067_P1_COIN);
        input_mux[i].p2_coin = !GETBIT(mux4067_vals_db[i], MUX4067_P2_COIN);

        input_mux[i].test = !GETBIT(mux4067_vals_db[i], MUX4067_TEST);
        input_mux[i].service = !GETBIT(mux4067_vals_db[i], MUX4067_SERVICE);
        input_mux[i].clear = !GETBIT(mux4067_vals_db[i], MUX4067_CLEAR);
    }
}

void input_task() {
    uint32_t current_ts = board_millis();

    mux4067_update(lights.p1_mux, lights.p2_mux);
    mux4067_debounce();

    update_input_mux();

    uint32_t merged = mux4067_merged(mux4067_vals_db);

    // read extra unused I/O for supported modes
    bool q = GETBIT(merged, MUX4067_JAMMA_17);
    if (q && !last_q) {
        q_toggle = !q_toggle;
    }

    jamma_w = GETBIT(merged, MUX4067_JAMMA_W);
    jamma_x = GETBIT(merged, MUX4067_JAMMA_X);
    jamma_y = GETBIT(merged, MUX4067_JAMMA_Y);
    jamma_z = GETBIT(merged, MUX4067_JAMMA_Z);

    if (config_mode) {
        if ((!input.p1_dl && last_input.p1_dl) || (!input.p2_dl && last_input.p2_dl)) {
            if (input_mode_tmp > 0)
                input_mode_tmp--;
        } else if ((!input.p1_dr && last_input.p1_dr) || (!input.p2_dr && last_input.p2_dr)) {
            if (input_mode_tmp < INPUT_MODE_COUNT - 1)
                input_mode_tmp++;
        } else if ((!input.test && last_input.test) || (!input.test && last_input.test)) {
            input_mode_tmp = (input_mode_tmp + 1) % input_mode_tmp;
        }
    }

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

    // enter usb bootloader mode (do not use in production!)
    if ((config_mode || ALWAYS_BOOTLOADER) && !input.p2_ul && !input.p2_ur && !input.p2_dr) {
        reset_usb_boot(0, 0);
    }

    last_input = input;
    last_q = q;
}

void config_mode_led_update(uint32_t* buf) {
    uint32_t time = board_millis();
    uint8_t blink_count = (time / SERVICE_BLINK_INTERVAL) % (input_mode + 1);
    // blink for BLINK_LENGTH every BLINK_INTERVAL ms for input_mode times.
    // then leave an empty spot at the end to separate the next blinking cycle
    bool state = (blink_count <= input_mode_tmp) && (time % SERVICE_BLINK_INTERVAL <= SERVICE_BLINK_LENGTH);

    SETORCLRBIT(*buf, LATCH_JAMMA_LED, state);

    SETORCLRBIT(*buf, LATCH_P1L_UPLEFT, state && (input_mode_tmp & 0b100));
    SETORCLRBIT(*buf, LATCH_P1L_CENTER, state && (input_mode_tmp & 0b10));
    SETORCLRBIT(*buf, LATCH_P1L_UPRIGHT, state && (input_mode_tmp & 0b1));

    SETORCLRBIT(*buf, LATCH_P2L_UPLEFT, state && (input_mode_tmp & 0b100));
    SETORCLRBIT(*buf, LATCH_P2L_CENTER, state && (input_mode_tmp & 0b10));
    SETORCLRBIT(*buf, LATCH_P2L_UPRIGHT, state && (input_mode_tmp & 0b1));

    SETORCLRBIT(*buf, LATCH_CABL_MARQ1, state && (input_mode_tmp & 0b100));
    SETORCLRBIT(*buf, LATCH_CABL_MARQ2, state && (input_mode_tmp & 0b10));
    SETORCLRBIT(*buf, LATCH_CABL_MARQ3, state && (input_mode_tmp & 0b1));
}

void lights_task() {
    #ifdef ENABLE_WS2812_SUPPORT
    ws2812_lock_mtx();
    #endif

    uint32_t buf = 0;

    if (config_mode) {
        // force direct_lights behavior
        uint32_t in_buf = mux4067_merged(mux4067_vals_db);
        
        SETORCLRBIT(buf, LATCH_P1L_DOWNLEFT, GETBIT(in_buf, MUX4067_P1_DOWNLEFT));
        SETORCLRBIT(buf, LATCH_P1L_DOWNRIGHT, GETBIT(in_buf, MUX4067_P1_DOWNRIGHT));

        SETORCLRBIT(buf, LATCH_P2L_DOWNLEFT, GETBIT(in_buf, MUX4067_P2_DOWNLEFT));
        SETORCLRBIT(buf, LATCH_P2L_DOWNRIGHT, GETBIT(in_buf, MUX4067_P2_DOWNRIGHT));

        SETORCLRBIT(buf, LATCH_P1_S0, lights.p1_mux & 0b1);
        SETORCLRBIT(buf, LATCH_P1_S1, lights.p1_mux & 0b10);
        SETORCLRBIT(buf, LATCH_P2_S0, lights.p2_mux & 0b1);
        SETORCLRBIT(buf, LATCH_P2_S1, lights.p2_mux & 0b10);

        SETBIT(buf, LATCH_CABL_MARQ4);
        SETBIT(buf, LATCH_CABL_NEON);

        SETBIT(buf, LATCH_ALWAYS_ON);

        config_mode_led_update(&buf);
        
    } else if (factory_test_mode) {
        buf = mux4067_merged(mux4067_vals_db);
    } else if (input_mode == INPUT_MODE_SERIAL) {
        buf = serial_lights_buf;
    } else if (direct_lights) {  // technically it could be direct_lights && !merge_mux
        uint32_t in_buf = mux4067_merged(mux4067_vals_db);

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
        
        SETORCLRBIT(buf, LATCH_CABL_MARQ1, lights.l1_halo);
        SETORCLRBIT(buf, LATCH_CABL_MARQ2, lights.l2_halo);
        SETORCLRBIT(buf, LATCH_CABL_MARQ3, lights.r1_halo);
        SETORCLRBIT(buf, LATCH_CABL_MARQ4, lights.r2_halo);
        SETORCLRBIT(buf, LATCH_CABL_NEON, lights.bass_light);

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

    if (auto_mux || config_mode) {
        current_mux = (current_mux + 1) % 4;
        lights.p1_mux = current_mux;
        lights.p2_mux = current_mux;
    }
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

    // move to cdc_task
    if (input_mode == INPUT_MODE_SERIAL) {
        if (tud_cdc_n_available(ITF_NUM_CDC_0)) {
            uint8_t buf[64] = {0};
            uint32_t count = tud_cdc_n_read(ITF_NUM_CDC_0, buf, sizeof(buf));
            
            if (count > 0) {
                int bit = atoi(buf);
                SETORCLRBIT(serial_lights_buf, bit, !GETBIT(serial_lights_buf, bit));
            }
        }

        uint32_t merged = mux4067_merged(mux4067_vals_db);
        for (int i = 31; i >= 0; i--) {
            tud_cdc_n_write_char(ITF_NUM_CDC_0, '0'+((merged >> i) & 1));
            if (i % 8 == 0) {
                tud_cdc_n_write_char(ITF_NUM_CDC_0, ',');
            }
        }
        tud_cdc_n_write_char(ITF_NUM_CDC_0, '\n');
        tud_cdc_n_write_flush(ITF_NUM_CDC_0);
        return;
    }

    // USB FEATURES : Send/Get USB Features (including Player LEDs on X-Input)
    void* report = NULL;
    uint16_t size = get_report(&report);
    send_report(report, size);
    receive_report(hid_rx_buf);
}

/*
char recv_buf[128] = "bottom text\n";

volatile host_message_t recv_host_msg = { .board_id='A', .s_loop_time="0123456789ABCDEF", .newline='\n'};
volatile client_message_t recv_client_msg = {.board_id='B', .s_loop_time="0123456789ABCDEF", .payload="un", .newline='\n'};
host_message_t last_host_msg;
client_message_t last_client_msg;
auto_init_mutex(uart_recv_mutex);

void uart_task() {
    if (input_mode == INPUT_MODE_SERIAL) {
        uint32_t mutex_owner;
        uint64_t start_ts = time_us_64();
        // loop until UART recv buffer is not in use by core 1, or until we time out (100 microseconds)
        while (mutex_try_enter(&uart_recv_mutex, &mutex_owner) && time_us_64() - start_ts < 100);

        // exit if we timed out
        if (time_us_64() - start_ts >= 100) {
            tud_cdc_n_write(ITF_NUM_CDC_0, "mutex timeout\n", 15);
            tud_cdc_n_write_flush(ITF_NUM_CDC_0);
            return;
        }

        if (UART_HOST && memcmp(recv_client_msg.data, last_client_msg.data, sizeof(recv_client_msg.data))) {
            tud_cdc_n_write(ITF_NUM_CDC_0, "start\n", 7);
            tud_cdc_n_write(ITF_NUM_CDC_0, recv_client_msg.data, sizeof(recv_client_msg.data));
            tud_cdc_n_write(ITF_NUM_CDC_0, "end\n", 5);
            tud_cdc_n_write_flush(ITF_NUM_CDC_0);
            memcpy(last_client_msg.data, recv_client_msg.data, sizeof(recv_client_msg.data));
        } else if (!UART_HOST && memcmp(recv_host_msg.data, last_host_msg.data, sizeof(recv_host_msg.data))) {
            tud_cdc_n_write(ITF_NUM_CDC_0, "start\n", 7);
            tud_cdc_n_write(ITF_NUM_CDC_0, recv_host_msg.data, sizeof(recv_host_msg.data));
            tud_cdc_n_write(ITF_NUM_CDC_0, "end\n", 5);
            tud_cdc_n_write_flush(ITF_NUM_CDC_0);
            memcpy(last_host_msg.data, recv_host_msg.data, sizeof(recv_host_msg.data));
        }   

        mutex_exit(&uart_recv_mutex);
    }
}

void rs485_write_blocking(uart_inst_t *uart, const uint8_t *src, size_t len) {
    gpio_put(UART_RE_PIN, 1);  // disable read while sending message over UART

    // uart_write_blocking(uart, src, len);
    sendMsg(uart, src, len);

    // uint32_t mutex_owner;
    // while (mutex_try_enter(&uart_recv_mutex, &mutex_owner));

    // snprintf(recv_client_msg.payload, sizeof(recv_client_msg.payload), "wr");  // i wrote

    // mutex_exit(&uart_recv_mutex);

    // maybe add a delay?
    // busy_wait_us(20);

    gpio_put(UART_RE_PIN, 0);  // enable read
}

uint8_t rs485_read_blocking(uart_inst_t *uart, const uint8_t *dst, size_t len) {
    //uart_read_blocking(uart, src, len);
    return recvMsg(uart, dst, len, 500);
}

void uart_core1_task() {
    static uint64_t last_ts = 0;
    uint64_t ts = time_us_64();
    uint32_t mutex_owner;

    if (UART_HOST) {
        host_message_t tx_message = { .board_id = UART_HOST_ID, .newline = '\n' };
        snprintf(tx_message.s_loop_time, sizeof(tx_message.s_loop_time), "%016x", ts - last_ts);

        last_ts = ts;

        rs485_write_blocking(uart0, tx_message.data, sizeof(tx_message.data));
        // sendMsg(tx_message.data, sizeof(tx_message.data));

        // wait for device to send reply message
        // uint64_t timeout_start_ts = time_us_64();
        while (!uart_is_readable(uart0));

        // if (time_us_64() - timeout_start_ts >= 100) {
        //     while (mutex_try_enter(&uart_recv_mutex, &mutex_owner));

        //     snprintf(recv_client_msg.payload, sizeof(recv_client_msg.payload), "to");  // timeout

        //     mutex_exit(&uart_recv_mutex);
        // } else {
        while (mutex_try_enter(&uart_recv_mutex, &mutex_owner));

        if (!rs485_read_blocking(uart0, recv_buf, sizeof(recv_client_msg.data))) {
            return;  // message not received so don't try to copy it
        }
        memcpy(recv_client_msg.data, recv_buf, sizeof(recv_client_msg.data));

        mutex_exit(&uart_recv_mutex);
        
    } else {
        while (!uart_is_readable(uart0));

        while (mutex_try_enter(&uart_recv_mutex, &mutex_owner));

        if (!rs485_read_blocking(uart0, recv_buf, sizeof(recv_host_msg.data))) {
            return;  // message not received so don't try to copy it
        }
        memcpy(recv_host_msg.data, recv_buf, sizeof(recv_host_msg.data));
        uint8_t recv_board_id = recv_host_msg.board_id;
    
        // check if last message received from host (board ID 0)
        if (recv_board_id == UART_HOST_ID) {
            
        } else {
            recv_host_msg.data[1] = '0'+(recv_host_msg.data[0] >> 8);
            recv_host_msg.data[2] = '0'+(recv_host_msg.data[0] % 16);
        }

        // then let's send back an acknowledgement message
            client_message_t ret_message = { .board_id = UART_DEVICE_ID, .payload = "ok", .newline = '\n' };

            snprintf(ret_message.s_loop_time, sizeof(ret_message.s_loop_time), "%016x", ts - last_ts);
            last_ts = ts;

            rs485_write_blocking(uart0, ret_message.data, sizeof(ret_message.data));

        mutex_exit(&uart_recv_mutex);
    }
}

void core1_entry() {
    gpio_init(UART_SHDN_PIN);
    gpio_init(UART_RE_PIN);
    gpio_set_dir(UART_SHDN_PIN, true);  // set to output
    gpio_set_dir(UART_RE_PIN, true);  // set to output

    uart_init(uart0, 9600);
    
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_hw_flow(uart0, false, false);

    // #define DATA_BITS 8
    // #define STOP_BITS 1
    // #define PARITY    UART_PARITY_NONE
    // uart_set_format(uart0, DATA_BITS, STOP_BITS, PARITY);

    gpio_put(UART_SHDN_PIN, 1);  // turn off shutdown by setting high

    while (true) {
        uart_core1_task();
    }
}
*/

void init() {
    get_input_mode();

    switch (input_mode) {
        case INPUT_MODE_PIUIO:
            direct_lights = false;
            auto_mux = false;
            merge_mux = false;
            break;

        case INPUT_MODE_GAMEPAD:
            direct_lights = true;
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

        case INPUT_MODE_GAMECUBE:
            direct_lights = true;
            auto_mux = true;
            merge_mux = true;
            break;

        case INPUT_MODE_SERIAL:
            direct_lights = true;
            auto_mux = true;
            merge_mux = true;
            break;
    }
}

int main() {
    board_init();

    // multicore_launch_core1(core1_entry);

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

        // uart_task();
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
    if (config_mode) return false;

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
            // if (report_type == HID_REPORT_TYPE_OUTPUT) {
            // do not consider the report type at all! tinyusb ignores it and sets it to HID_REPORT_TYPE_INVALID (0)
            // also note that no report ID is specified in the LXIO's device descriptor
            lxio_set_report(buffer, bufsize, &lights);
        } else if (input_mode == INPUT_MODE_GAMECUBE) {
            // rumble
        }
    }
}