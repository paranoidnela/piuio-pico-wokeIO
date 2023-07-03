/******************************************************************************/
/*  SPDX-License-Identifier: MIT                                              */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 sugoku                         */
/*  SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com) */
/*  https://github.com/sugoku/piuio-pico-brokeIO                              */
/******************************************************************************/

#ifndef _SWITCH_REPORT_H
#define _SWITCH_REPORT_H

#include "piuio_config.h"

#define SWITCH_ENDPOINT_SIZE 64

// HAT report (4 bits)
#define SWITCH_HAT_UP        0x00
#define SWITCH_HAT_UPRIGHT   0x01
#define SWITCH_HAT_RIGHT     0x02
#define SWITCH_HAT_DOWNRIGHT 0x03
#define SWITCH_HAT_DOWN      0x04
#define SWITCH_HAT_DOWNLEFT  0x05
#define SWITCH_HAT_LEFT      0x06
#define SWITCH_HAT_UPLEFT    0x07
#define SWITCH_HAT_NOTHING   0x08

// Button report (16 bits)
#define SWITCH_MASK_Y       (1U <<  0)
#define SWITCH_MASK_B       (1U <<  1)
#define SWITCH_MASK_A       (1U <<  2)
#define SWITCH_MASK_X       (1U <<  3)
#define SWITCH_MASK_L       (1U <<  4)
#define SWITCH_MASK_R       (1U <<  5)
#define SWITCH_MASK_ZL      (1U <<  6)
#define SWITCH_MASK_ZR      (1U <<  7)
#define SWITCH_MASK_MINUS   (1U <<  8)
#define SWITCH_MASK_PLUS    (1U <<  9)
#define SWITCH_MASK_L3      (1U << 10)
#define SWITCH_MASK_R3      (1U << 11)
#define SWITCH_MASK_HOME    (1U << 12)
#define SWITCH_MASK_CAPTURE (1U << 13)

// Switch analog sticks only report 8 bits
#define SWITCH_JOYSTICK_MIN 0x00
#define SWITCH_JOYSTICK_MID 0x80
#define SWITCH_JOYSTICK_MAX 0xFF

typedef struct __attribute((packed, aligned(1)))
{
	uint16_t buttons;
	uint8_t hat;
	uint8_t lx;
	uint8_t ly;
	uint8_t rx;
	uint8_t ry;
	uint8_t vendor;
} SwitchReport;

typedef struct
{
	uint16_t buttons;
	uint8_t hat;
	uint8_t lx;
	uint8_t ly;
	uint8_t rx;
	uint8_t ry;
} SwitchOutReport;

static SwitchReport switchReport = {
	.buttons = 0,
	.hat = SWITCH_HAT_NOTHING,
	.lx = SWITCH_JOYSTICK_MID,
	.ly = SWITCH_JOYSTICK_MID,
	.rx = SWITCH_JOYSTICK_MID,
	.ry = SWITCH_JOYSTICK_MID,
	.vendor = 0,
};

uint16_t switch_get_report(SwitchReport** report, struct inputArray* input) {
	#if !defined(SWITCH_JOYSTICK)
	switchReport.hat = SWITCH_HAT_NOTHING;  
	if (!input->p1_cn) switchReport.hat = SWITCH_HAT_UP;
	if (!input->p1_cn && !input->p1_ur) switchReport.hat = SWITCH_HAT_UPRIGHT;  
	if (!input->p1_ur) switchReport.hat = SWITCH_HAT_RIGHT;    
	if (!input->p1_ur && !input->p1_dr) switchReport.hat = SWITCH_HAT_DOWNRIGHT;
	if (!input->p1_dr) switchReport.hat = SWITCH_HAT_DOWN;     
	if (!input->p1_dr && !input->p1_dl) switchReport.hat = SWITCH_HAT_DOWNLEFT; 
	if (!input->p1_dl) switchReport.hat = SWITCH_HAT_LEFT;     
	if (!input->p1_dl && !input->p1_cn) switchReport.hat = SWITCH_HAT_UPLEFT;
	#else
	switchReport.lx = SWITCH_JOYSTICK_MID;
	switchReport.ly = SWITCH_JOYSTICK_MID;
	// note that the Y direction is flipped
	if (!input->p1_cn) switchReport.ly = SWITCH_JOYSTICK_MIN;
	if (!input->p1_ur) switchReport.lx = SWITCH_JOYSTICK_MAX;
	if (!input->p1_dr) switchReport.ly = SWITCH_JOYSTICK_MAX;     
	if (!input->p1_dl) switchReport.lx = SWITCH_JOYSTICK_MIN;
	#endif

	switchReport.buttons = 0
		| (!input->p2_dl ? SWITCH_MASK_B       : 0)
		| (!input->p2_dr ? SWITCH_MASK_A       : 0)
		| (!input->p2_ul ? SWITCH_MASK_Y       : 0)
		| (!input->p2_cn ? SWITCH_MASK_X       : 0)
		| (!input->p1_ul ? SWITCH_MASK_L       : 0)
		| (!input->p2_ur ? SWITCH_MASK_R       : 0)
		// | (pressedL2() ? SWITCH_MASK_ZL      : 0)
		// | (pressedR2() ? SWITCH_MASK_ZR      : 0)
		| (!input->p1_coin ? SWITCH_MASK_MINUS   : 0)
		| (!input->p2_coin ? SWITCH_MASK_PLUS    : 0)
		// | (pressedL3() ? SWITCH_MASK_L3      : 0)
		// | (pressedR3() ? SWITCH_MASK_R3      : 0)
		| (!input->test ? SWITCH_MASK_HOME    : 0)
		| (!input->service ? SWITCH_MASK_CAPTURE : 0)
	;

	//switchReport.lx = static_cast<uint8_t>(state.lx >> 8);
	//switchReport.ly = static_cast<uint8_t>(state.ly >> 8);
	//switchReport.rx = static_cast<uint8_t>(state.rx >> 8);
	//switchReport.ry = static_cast<uint8_t>(state.ry >> 8);

	*report = &switchReport;
	return sizeof(SwitchReport);
}

#endif