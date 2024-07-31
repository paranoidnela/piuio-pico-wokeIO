/**********************************************************/
/*  SPDX-License-Identifier: MIT                          */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 sugoku     */
/*  https://github.com/sugoku/piuio-pico-brokeIO          */
/**********************************************************/

#ifndef _PIUIO_DESC_H
#define _PIUIO_DESC_H

#include "tusb.h"

enum
{
    ITF_NUM_VENDOR,
    ITF_NUM_TOTAL
};

#define PIUIO_CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_LEN)

#define EPNUM_VENDOR_IN 0x01
#define EPNUM_VENDOR_OUT 0x01

static const uint8_t piuio_string_language[]    = { 0x09, 0x04 };
static const uint8_t piuio_string_manufacturer[] = "sugoku";
static const uint8_t piuio_string_product[]     = "brokeIO RP2040 (PIUIO)";
static const uint8_t piuio_string_version[]     = "727";
static const uint8_t piuio_string_vendor_interface[] = "piuio-pico-brokeIO";

static const uint8_t *piuio_string_descriptors[] =
{
    piuio_string_language,
    piuio_string_manufacturer,
    piuio_string_product,
    piuio_string_version,
        piuio_string_vendor_interface
};

tusb_desc_device_t const piuio_device_descriptor =
        {
                .bLength            = sizeof(tusb_desc_device_t),
                .bDescriptorType    = TUSB_DESC_DEVICE,
                .bcdUSB             = 0x0100, // at least 2.1 or 3.x for BOS & webUSB

                // Use Interface Association Descriptor (IAD) for CDC
                // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
                .bDeviceClass       = 0xFF,
                .bDeviceSubClass    = 0x00,
                .bDeviceProtocol    = 0x00,
                .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

                .idVendor           = 0x0547,
                .idProduct          = 0x1002,
                .bcdDevice          = 0x0100,

                .iManufacturer      = 0x01,
                .iProduct           = 0x02,
                .iSerialNumber      = 0x03,

                .bNumConfigurations = 0x01
        };

static const uint8_t piuio_configuration_descriptor[] =
        {
                // Config number, interface count, string index, total length, attribute, power in mA
                TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, PIUIO_CONFIG_TOTAL_LEN, 0x00, 500),

                // Interface number, string index, EP Out & IN address, EP size
                TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR, 4, EPNUM_VENDOR_OUT, 0x80 | EPNUM_VENDOR_IN,
                                      TUD_OPT_HIGH_SPEED ? 512 : 64)
        };

#endif