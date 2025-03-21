/*******************************************************************************************/
/*  SPDX-License-Identifier: MIT                                                           */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 48productions, therathatter, dj505, sugoku  */
/*  https://github.com/sugoku/piuio-pico-brokeIO                                           */
/*******************************************************************************************/

#ifndef _PIUIO_CONFIG_H
#define _PIUIO_CONFIG_H

#include "hardware/timer.h"

#include "usb_descriptors.h"
#include "usb_hid_keys.h"

// debounce time in milliseconds
// adjust if you are getting misfires!
// does nothing rn, debounce needs to be implemented
#define DEBOUNCE_PRESS_TIME 15
#define DEBOUNCE_RELEASE_TIME 15

// enable debouncing
#define DEBOUNCING

// use joystick instead of d-pad in Switch mode
#define SWITCH_JOYSTICK

// merge all sensor inputs, improving response time at the cost of not being able
// to read individual sensors in the service menu.
// in PIUIO mode, one poll reads one sensor per panel (10/40 sensors per poll).
// official games only poll the PIUIO every 10ms, which is 100Hz best case but
// 25Hz worst case the game is reading the wrong sensor.
// setting this to true will improve sensor press polling to always be 100Hz,
// but release polling will still be 25Hz worst case.
#define MERGE_MUX_PIUIO false

// time in microseconds to wait between input/lights operations
// this accounts for variance in the brokeIO's multiplexer and latch chips
// the delay is negligible, so you would not notice it
//#define WAIT_INPUT_MUX4067 20
//#define WAIT_LIGHTS_LATCH32 20

// always allow pad combo to enter bootloader; otherwise, it must be done in the service mode
#define ALWAYS_BOOTLOADER false

// default input mode unless otherwise specified in the flash memory
#define DEFAULT_INPUT_MODE INPUT_MODE_PIUIO

// use software SPI to control latch for outputs
// for some reason hardware SPI wasn't working right for me so I have it enabled
//#define SOFTWARE_LATCH

// toggle pin on and off on the main loop for debugging purposes
// #define BENCHMARK

// uncomment to always use the default input mode on boot instead of what's in the flash memory
// disables reading/writing to flash also
// (you will not be able to change the mode until reflashing!)
// #define ALWAYS_DEFAULT_INPUT_MODE

// threshold in ms to hold SERVICE button to enter mode select (settings menu)
#define SETTINGS_THRESHOLD 2000

// these PIDs are granted by Openmoko! they are not used for PIUIO/LXIO modes
// https://github.com/openmoko/openmoko-usb-oui
#define VENDOR_ID               0x1D50
#define PRODUCT_ID_GAMEPAD      0x6181
#define PRODUCT_ID_KEYBOARD     0x6182
#define PRODUCT_ID_OTHER        0x6183

// enable pullup resistors for inputs
// (only disable this if you know what you are doing!)
//#define PULLUP_IN

#define MUX_GLOBAL 4
#define MUX_COUNT 5

#define MAX_USB_POWER 0xFA  // (500mA)


// helper defines

#define GETBIT(port,bit) ((port) & (1 << (bit)))     // get value at bit
#define SETBIT(port,bit) ((port) |= (1 << (bit)))    // set bit to 1
#define CLRBIT(port,bit) ((port) &= ~(1 << (bit)))   // set bit to 0 (clear bit)
#define SETORCLRBIT(port,bit,val) if (val) { SETBIT(port,bit); } else { CLRBIT(port,bit); }  // if true, set bit to 1, if false, clear bit to 0

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

// other defines
// offset from XIP_BASE, let's make it 1MiB from the start
#define INPUT_MODE_OFFSET (1024 * 1024)



// UART defines
//#define UART_HOST true
//#define UART_HOST_ID '0'
//#define UART_DEVICE_ID '1'


// HID defines

// in lane order: zqsec 17593

#define KEYCODE_P1_UPLEFT KEY_Q
#define KEYCODE_P1_UPRIGHT KEY_E
#define KEYCODE_P1_CENTER KEY_S
#define KEYCODE_P1_DOWNLEFT KEY_Z
#define KEYCODE_P1_DOWNRIGHT KEY_C

#define KEYCODE_P2_UPLEFT KEY_KP7
#define KEYCODE_P2_UPRIGHT KEY_KP9
#define KEYCODE_P2_CENTER KEY_KP5
#define KEYCODE_P2_DOWNLEFT KEY_KP1
#define KEYCODE_P2_DOWNRIGHT KEY_KP3

// F5, F6
#define KEYCODE_P1_COIN KEY_F5
#define KEYCODE_P2_COIN KEY_F6

// F2, F9, F1
#define KEYCODE_TEST KEY_F2
#define KEYCODE_SERVICE KEY_F9
#define KEYCODE_CLEAR KEY_F1


#endif //_PIUIO_CONFIG_H
