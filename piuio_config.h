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
#define DEBOUNCE_COUNT 50

// enable debouncing
#define DEBOUNCING true

// use joystick instead of d-pad in Switch mode
#define SWITCH_JOYSTICK

// always allow pad combo to enter bootloader; otherwise, it must be done in the service mode
#define ALWAYS_BOOTLOADER false

// default input mode unless otherwise specified in the flash memory
#define DEFAULT_INPUT_MODE INPUT_MODE_PIUIO

// set HID Gamepad mode to L-TEK layout instead of brokeIO defaults for support with existing hooks
#define GAMEPAD_LTEK_MODE true

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
