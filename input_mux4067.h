/**********************************************************/
/*  SPDX-License-Identifier: MIT                          */
/*  https://github.com/sugoku/piuio-pico-brokeIO          */
/**********************************************************/

#ifndef _INPUT_MUX4067_H
#define _INPUT_MUX4067_H

#include "piuio_config.h"

extern uint32_t mux4067_vals[5];  // 1 for each MUX state + global, we are caching them so we can poll faster even when the PC isn't asking

extern const uint8_t mux_p1_inputs[];
extern const uint8_t mux_p2_inputs[];

void mux4067_init();
void mux4067_update(uint8_t mux_p1, uint8_t mux_p2);  // read from multiplexers and take in a value `mux` which determines which sensor we are reading from
void mux4067_enable();  // enable the multiplexer
void mux4067_disable();  // disable the multiplexer
void mux4067_reset();  // set all read inputs to an off state, reset all selector pins, disable the multiplexer
uint32_t mux4067_merged(uint32_t* vals);

#endif