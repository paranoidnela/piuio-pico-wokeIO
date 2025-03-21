/******************************************************************************/
/*  SPDX-License-Identifier: MIT                                              */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 sugoku                         */
/*  SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com) */
/*  https://github.com/sugoku/piuio-pico-brokeIO                              */
/******************************************************************************/

#ifndef _KEYBOARD_DESC_H
#define _KEYBOARD_DESC_H

#include <stdint.h>
#include "tusb.h"
#include "../piuio_config.h"

#define KEY_COUNT 104

static const uint8_t keyboard_string_language[]    = { 0x09, 0x04 };
static const uint8_t keyboard_string_manufacturer[] = "nela";
static const uint8_t keyboard_string_product[]     = "wokeIO (Keyboard)";
static const uint8_t keyboard_string_version[]     = "727";

static const uint8_t *keyboard_string_descriptors[] =
{
    keyboard_string_language,
    keyboard_string_manufacturer,
    keyboard_string_product,
    keyboard_string_version
};

static const uint8_t keyboard_device_descriptor[] =
{
    sizeof(tusb_desc_device_t),    // bLength
    TUSB_DESC_DEVICE,            // bDescriptorType
    0x00, 0x02,                    // bcdUSB 2.00
    0x00,                        // bDeviceClass
    0x00,                        // bDeviceSubClass
    0x00,                        // bDeviceProtocol
    64,                            // bMaxPacketSize0
    LSB(VENDOR_ID), MSB(VENDOR_ID),      // idVendor
    LSB(PRODUCT_ID_KEYBOARD), MSB(PRODUCT_ID_KEYBOARD), // idProduct
    0x00, 0x01,                    // bcdDevice
    0x01,                        // iManufacturer
    0x02,                        // iProduct
    0x00,                        // iSerialNumber
    0x01                        // bNumConfigurations
};

enum
{
    ITF_NUM_HID_KEYBOARD,
    ITF_NUM_TOTAL_KEYBOARD
};

#define KEYBOARD_CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID   0x81

static const uint8_t keyboard_report_descriptor[] = 
{
    0x05, 0x01,                // Usage Page (Generic Desktop),
    0x09, 0x06,                // Usage (Keyboard),
    0xA1, 0x01,                // Collection (Application),
    // Modifier Keys
    0x05, 0x07,                    // Usage Page (Key Codes),
    0x19, 0xE0,                    // Usage Minimum (224),
    0x29, 0xE7,                    // Usage Maximum (231),
    0x15, 0x00,                    // Logical Minimum (0),
    0x25, 0x01,                    // Logical Maximum (1),
    0x75, 0x01,                    // Report Size (1),
    0x95, 0x08,                    // Report Count (8),
    0x81, 0x02,                    // Input (Data, Variable, Absolute),                ;Modifier byte (0)
    // LEDS
    0x05, 0x08,                    // Usage Page (LEDs),
    0x19, 0x01,                    // Usage Minimum (1),
    0x29, 0x05,                    // Usage Maximum (5),
    0x75, 0x01,                    // Report Size (1),
    0x95, 0x05,                    // Report Count (5),
    0x91, 0x02,                    // Output (Data, Variable, Absolute),                ;LED Report (5/8)
    0x75, 0x03,                    // Report Size (3),
    0x95, 0x01,                    // Report Count (1),
    0x91, 0x03,                    // Output (Constant, Variable, Absolute),            ;LED Report padding (8/8)
    // Keys
    0x05, 0x07,                    // Usage Page (Key Codes),
    0x19, 0x00,                    // Usage Minimum (0),
    0x29, KEY_COUNT - 1,        // Usage Maximum (103),
    0x15, 0x00,                    // Logical Minimum (0),
    0x25, 0x01,                    // Logical Maximum (1),
    0x75, 0x01,                    // Report Size (1),
    0x95, KEY_COUNT,            // Report Count (104),
    0x81, 0x02,                    // Input (Data, Variable, Absolute),                ;Key byte (1-13)
    0x75, 0x08,                    // Report Size (8),
    0x95, 0x02,                    // Report Count (2),
    0x81, 0x03,                    // Input (Constant, Variable, Absolute),            ;Key byte padding (14-15)
    0xC0                    // End Collection
};

// Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
static const uint8_t keyboard_hid_descriptor[] =
{
    0x09,                                       // bLength
    0x21,                                       // bDescriptorType (HID)
    0x11, 0x01,                                   // bcdHID 1.11
    0x00,                                       // bCountryCode
    0x01,                                      // bNumDescriptors
    0x22,                                      // bDescriptorType[0] (HID)
    sizeof(keyboard_report_descriptor), 0x00, // wDescriptorLength[0] 90
};

static const uint8_t keyboard_configuration_descriptor[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL_KEYBOARD, 0, KEYBOARD_CONFIG_TOTAL_LEN, 32, 500),

    // Interface number, string index, protocol, report descriptor len, EP Out & In address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID_KEYBOARD, 0, HID_ITF_PROTOCOL_KEYBOARD, sizeof(keyboard_report_descriptor), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 1)
};

#endif