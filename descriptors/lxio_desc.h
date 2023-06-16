/******************************************************************************/
/*  SPDX-License-Identifier: MIT                                              */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 sugoku                         */
/*  SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com) */
/*  https://github.com/sugoku/piuio-pico-brokeIO                              */
/******************************************************************************/

#ifndef _LXIO_DESC_H
#define _LXIO_DESC_H

#include "../piuio_config.h"
#include "tusb.h"

static const uint8_t lxio_string_language[]    = { 0x09, 0x04 };
static const uint8_t lxio_string_manufacturer[] = "sugoku";
static const uint8_t lxio_string_product[]     = "brokeIO (LXIO)";
static const uint8_t lxio_string_version[]     = "727";

static const uint8_t *lxio_string_descriptors[] =
{
    lxio_string_language,
    lxio_string_manufacturer,
    lxio_string_product,
    lxio_string_version
};

tusb_desc_device_t const lxio_device_descriptor =
        {
                .bLength            = sizeof(tusb_desc_device_t),
                .bDescriptorType    = TUSB_DESC_DEVICE,
                .bcdUSB             = 0x0200, // USB 2.0

                // Use Interface Association Descriptor (IAD) for CDC
                // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
                .bDeviceClass       = 0x00,
                .bDeviceSubClass    = 0x00,
                .bDeviceProtocol    = 0x00,
                .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

                .idVendor           = 0x0D2F,
                .idProduct          = 0x1020,
                .bcdDevice          = 0x0001,

                .iManufacturer      = 0x01,
                .iProduct           = 0x02,
                .iSerialNumber      = 0x03,

                .bNumConfigurations = 0x01
        };

static const uint8_t lxio_report_descriptor[] =
{
    0x06, 0x00, 0xff,   // Usage Page (Vendor)
    0x09, 0x01,         // Usage (Vendor)
    0xa1, 0x01,         // Collection (Application)
    0x09, 0x02,         //   Usage (Vendor)
    0x15, 0x00,         //   Logical Minimum (0)
    0x25, 0xff,         //   Logical Maximum (255)
    0x75, 0x08,         //   Report Size (8)
    0x95, 0x10,         //   Report Count (16)
    0x81, 0x02,         //   Input (Data,Var,Abs)
    0x09, 0x03,         //   Usage (Vendor)
    0x15, 0x00,         //   Logical Minimum (0)
    0x25, 0xff,         //   Logical Maximum (255)
    0x75, 0x08,         //   Report Size (8)
    0x95, 0x10,         //   Report Count (16)
    0x91, 0x02,         //   Output (Data,Var,Abs)
    0xc0,               // End Collection
};

static const uint8_t lxio_configuration_descriptor[] =
{
        // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
    9,                               // bLength;
    2,                               // bDescriptorType;
    0x29, 0x00,                      // wTotalLength 41
    1,                               // bNumInterfaces
    1,                               // bConfigurationValue
    0,                               // iConfiguration
    0x80,                            // bmAttributes
    MAX_USB_POWER,                   // bMaxPower
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
    9,                               // bLength
    4,                               // bDescriptorType
    0,                               // bInterfaceNumber
    0,                               // bAlternateSetting
    2,                               // bNumEndpoints
    0x03,                            // bInterfaceClass (0x03 = HID)
    0x00,                            // bInterfaceSubClass (0x00 = No Boot)
    0x00,                            // bInterfaceProtocol (0x00 = No Protocol)
    0,                               // iInterface
        // HID interface descriptor, HID 1.11 spec, section 6.2.1
    9,                               // bLength
    0x21,                            // bDescriptorType
    0x11, 0x01,                      // bcdHID
    0,                               // bCountryCode
    1,                               // bNumDescriptors
    0x22,                            // bDescriptorType
    sizeof(lxio_report_descriptor),  // wDescriptorLength
    0,
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                               // bLength
    5,                               // bDescriptorType
    0x81,                            // bEndpointAddress
    0x03,                            // bmAttributes (0x03=intr)
    0x10, 0,                         // wMaxPacketSize
    1,                               // bInterval (1 ms)
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
    7,                                // bLength
    5,                                // bDescriptorType
    0x02,                             // bEndpointAddress
    0x03,                             // bmAttributes (0x03=intr)
    0x40, 0,                          // wMaxPacketSize
    1                                 // bInterval (1 ms)
};

#endif