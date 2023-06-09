/**********************************************************/
/*  SPDX-License-Identifier: MIT                          */
/*  https://github.com/sugoku/piuio-pico-brokeIO          */
/**********************************************************/

#ifndef PIUIO_PICO_PIUIO_CONFIG_H
#define PIUIO_PICO_PIUIO_CONFIG_H
#include "piuio_ws2812_helpers.h"

#include "hardware/timer.h"

#include "usb_descriptors.h"

// helpful regex
// CLRBIT\(\w+, (\w+)\)
// gpio_put($1, 0)
// SETBIT\(\w+, (\w+)\)
// gpio_put($1, 1)
// SETORCLRBIT\(.+, (.+), (.+)\)
// gpio_put($1, $2)
// (\w+.\w+) = buf\[(\w+)\]
// SETORCLRBIT(buf, $2, $1)

// helper defines

#define GETBIT(port,bit) ((port) & (1 << (bit)))     // get value at bit
#define SETBIT(port,bit) ((port) |= (1 << (bit)))    // set bit to 1
#define CLRBIT(port,bit) ((port) &= ~(1 << (bit)))   // set bit to 0 (clear bit)
#define SETORCLRBIT(port,bit,val) if (val) { SETBIT(port,bit); } else { CLRBIT(port,bit); }  // if true, set bit to 1, if false, clear bit to 0

// use software SPI to control latch for outputs
// for some reason hardware SPI wasn't working right for me so I have it enabled
#define SOFTWARE_LATCH

// enable pullup resistors for inputs
// (only disable this if you know what you are doing!)
#define PULLUP_IN

#define MUX_GLOBAL 4

// default input mode unless otherwise specified in the flash memory
#define DEFAULT_INPUT_MODE INPUT_MODE_PIUIO

#define MAX_USB_POWER 0xFA  // (500mA)

// threshold in ms to hold SERVICE button to enter mode select (settings menu)
#define SETTINGS_THRESHOLD 2000

// Uncomment these defines to enable WS2812 LED support.
//#define ENABLE_WS2812_SUPPORT
//#define WS2812_IS_RGBW false
//#define WS2812_PIN 22

// Modify these to edit the colors of the cabinet lamps.
static uint32_t ws2812_color[5] = {
        urgb_u32(0, 255, 0),    // Lower left
        urgb_u32(255, 0, 0),    // Upper left
        urgb_u32(0, 0, 255),    // Bass / neon
        urgb_u32(255, 0, 0),    // Upper right
        urgb_u32(0, 255, 0)     // Lower right
};

// Modify these arrays to edit the pin out.
// Map these according to your button pins.
#define MUX4067_P1_UPLEFT 23
#define MUX4067_P1_UPRIGHT 22
#define MUX4067_P1_CENTER 21
#define MUX4067_P1_DOWNLEFT 20
#define MUX4067_P1_DOWNRIGHT 19

#define MUX4067_P2_UPLEFT 4
#define MUX4067_P2_UPRIGHT 3
#define MUX4067_P2_CENTER 2
#define MUX4067_P2_DOWNLEFT 1
#define MUX4067_P2_DOWNRIGHT 0

#define MUX4067_P1_COIN 29
#define MUX4067_P2_COIN 10

#define MUX4067_TEST 30
#define MUX4067_SERVICE 25
#define MUX4067_CLEAR 24

// Map these according to your LED pins.
#define LATCH_P1L_UPLEFT 29
#define LATCH_P1L_UPRIGHT 28
#define LATCH_P1L_CENTER 27
#define LATCH_P1L_DOWNLEFT 26
#define LATCH_P1L_DOWNRIGHT 25

#define LATCH_P2L_UPLEFT 13
#define LATCH_P2L_UPRIGHT 12
#define LATCH_P2L_CENTER 11
#define LATCH_P2L_DOWNLEFT 10
#define LATCH_P2L_DOWNRIGHT 9

#define LATCH_P1_S0 31
#define LATCH_P1_S1 30
#define LATCH_P2_S0 15
#define LATCH_P2_S1 14

#define LATCH_CABL_MARQ1 6
#define LATCH_CABL_MARQ2 8
#define LATCH_CABL_MARQ3 7
#define LATCH_CABL_MARQ4 5
#define LATCH_CABL_NEON 21

#define LATCH_COIN_COUNTER 3
#define LATCH_ALWAYS_ON 4

#define LATCH_JAMMA_LED 20


// other pins
#define MUX_ENABLE_PIN 21
#define MUX1_IN_PIN 26
#define MUX2_IN_PIN 27

#define MUX_S0_PIN 22
#define MUX_S1_PIN 23
#define MUX_S2_PIN 24
#define MUX_S3_PIN 25

#define LATCH_ENABLE_PIN 20
#define LATCH_RST_PIN 19
#define LATCH_RCLK_PIN 18

#define SOFTWARE_SPI_DIN_PIN 9
#define SOFTWARE_SPI_CLK_PIN 8


// other defines
// offset from XIP_BASE, let's make it 1MiB from the start
#define INPUT_MODE_OFFSET (1024 * 1024)
// turn LED on for 200ms every 400ms 
#define SERVICE_BLINK_LENGTH 200
#define SERVICE_BLINK_INTERVAL 400


// HID defines

#define KEYCODE_P1_UPLEFT 81
#define KEYCODE_P1_UPRIGHT 69
#define KEYCODE_P1_CENTER 83
#define KEYCODE_P1_DOWNLEFT 90
#define KEYCODE_P1_DOWNRIGHT 67

#define KEYCODE_P2_UPLEFT 103
#define KEYCODE_P2_UPRIGHT 105
#define KEYCODE_P2_CENTER 101
#define KEYCODE_P2_DOWNLEFT 97
#define KEYCODE_P2_DOWNRIGHT 99

// F5, F6
#define KEYCODE_P1_COIN 116
#define KEYCODE_P2_COIN 117

// F2, F9, F1
#define KEYCODE_TEST 113
#define KEYCODE_SERVICE 120
#define KEYCODE_CLEAR 112


#endif //PIUIO_PICO_PIUIO_CONFIG_H
