/*******************************************************************************************/
/*  SPDX-License-Identifier: MIT                                                           */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 48productions, therathatter, dj505, sugoku  */
/*  https://github.com/sugoku/piuio-pico-brokeIO                                           */
/*******************************************************************************************/

#ifndef _PIUIO_WS2812_H
#define _PIUIO_WS2812_H

#include "piuio_structs.h"

void ws2812_init(struct lampArray* l);
void ws2812_lock_mtx();
void ws2812_unlock_mtx();

#endif //_PIUIO_WS2812_H
