/******************************************************************************/
/* SPDX-License-Identifier: MIT                                               */
/* SPDX-FileCopyrightText: Copyright (c) 2023 sugoku                          */
/* SPDX-FileCopyrightText: Copyright (c) 2022 arpruss                         */
/* SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)  */
/* https://github.com/sugoku/piuio-pico-brokeIO                               */
/******************************************************************************/

#ifndef _GAMECUBE_DESC_H
#define _GAMECUBE_DESC_H

#include <stdint.h>
#include "../piuio_config.h"

#define GAMECUBE_ENDPOINT_SIZE 64

static const uint8_t gamecube_string_language[]     = { 0x09, 0x04 };
static const uint8_t gamecube_string_manufacturer[] = "sugoku";
static const uint8_t gamecube_string_product[]      = "brokeIO (GameCube)";
static const uint8_t gamecube_string_version[]      = "727";

static const uint8_t *gamecube_string_descriptors[] =
{
    gamecube_string_language,
    gamecube_string_manufacturer,
    gamecube_string_product,
    gamecube_string_version
};

static const uint8_t gamecube_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass
    0x00,        // bDeviceProtocol
    0x40,        // bMaxPacketSize0 64
    0x7E,
    0x05,      // idVendor 0x057E
    0x37,
    0x03,      // idProduct 0x0337
    0x00, 0x01,  // bcdDevice 2.00
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x03,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

static const uint8_t gamecube_report_descriptor[] =
{
    0x05, 0x05,        // Usage Page (Game Ctrls)
    0x09, 0x00,        // Usage (Undefined)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x11,        //   Report ID (17) RUMBLE
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x05,        //   Report Count (5)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x21,        //   Report ID (33) MAIN
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x25,        //   Report Count (37)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x12,        //   Report ID (18)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x22,        //   Report ID (34)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x19,        //   Report Count (25)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x13,        //   Report ID (19)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x23,        //   Report ID (35)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x14,        //   Report ID (20)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x24,        //   Report ID (36)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x15,        //   Report ID (21)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x00,        //   Output (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x25,        //   Report ID (37)
    0x19, 0x00,        //   Usage Minimum (Undefined)
    0x2A, 0xFF, 0x00,  //   Usage Maximum (0xFF)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};

static const uint8_t gamecube_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x29, 0x00,  // wTotalLength 41
    0x01,        // bNumInterfaces 1
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0x80,        // bmAttributes
    MAX_USB_POWER,        // bMaxPower 500mA

    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints 2
    0x03,        // bInterfaceClass
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    sizeof(gamecube_report_descriptor), 0x00,  // wDescriptorLength[0] 86

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x02,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)
};

static const uint8_t gamecube_hid_descriptor[] =
{
    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    sizeof(gamecube_report_descriptor), 0x00,  // wDescriptorLength[0] 86
};

#endif