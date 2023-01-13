/**********************************************************/
/*  SPDX-License-Identifier: MIT                          */
/*  https://github.com/sugoku/piuio-pico-brokeIO          */
/**********************************************************/

#ifndef _LIGHTS_LATCH32_H
#define _LIGHTS_LATCH32_H

#include "hardware/pio.h"
#include "hardware/structs/spi.h"

#include "sspi.pio.h"
#include "piuio_config.h"

// software SPI clock divider
// it was originally at 1.f and that was too fast
#define SERIAL_CLK_DIV 8.f

extern PIO pio;
extern uint sm;

extern uint8_t last_mux;
void lights_init();
void lights_send(uint32_t* buf);
void lights_enable();
void lights_disable();
void lights_reset();

#endif