/**********************************************************************/
/*  SPDX-License-Identifier: MIT                                      */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 sugoku                 */
/*  SPDX-FileCopyrightText: Copyright (c) 2019 Ha Thach (tinyusb.org) */
/*  https://github.com/sugoku/piuio-pico-brokeIO                      */
/**********************************************************************/

#ifndef _SERIAL_DESC_H
#define _SERIAL_DESC_H

#include "tusb.h"
#include "../piuio_config.h"

enum
{
  ITF_NUM_CDC_0 = 0,
  ITF_NUM_CDC_0_DATA,
  ITF_NUM_TOTAL_SERIAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN)

static const uint8_t serial_string_language[]    = { 0x09, 0x04 };
static const uint8_t serial_string_manufacturer[] = "nela";
static const uint8_t serial_string_product[]     = "wokeIO RP2040 (Serial)";
static const uint8_t serial_string_version[]     = "727";

static const uint8_t *serial_string_descriptors[] =
{
    serial_string_language,
    serial_string_manufacturer,
    serial_string_product,
    serial_string_version
};

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const serial_device_descriptor =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0110,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = VENDOR_ID,
    .idProduct          = PRODUCT_ID_OTHER,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

#define EPNUM_CDC_0_NOTIF   0x81
#define EPNUM_CDC_0_OUT     0x02
#define EPNUM_CDC_0_IN      0x82

static const uint8_t serial_configuration_descriptor[] =
{
  // Config number, interface count, string index, total length, attribute, power in mA
  TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL_SERIAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

  // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
  TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_OUT, EPNUM_CDC_0_IN, 64)
};

#endif