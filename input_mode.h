/**********************************************************/
/*  SPDX-License-Identifier: MIT                          */
/*  https://github.com/sugoku/piuio-pico-brokeIO          */
/**********************************************************/

#ifndef _INPUT_MODE_H
#define _INPUT_MODE_H

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "piuio_config.h"

extern int input_mode;

int get_input_mode();
uint8_t read_input_mode();
void write_input_mode(uint8_t value);

#endif