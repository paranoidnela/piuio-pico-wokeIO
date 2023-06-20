/******************************************************************************/
/*  SPDX-License-Identifier: MIT                                              */
/*  SPDX-FileCopyrightText: Copyright (c) 2023 sugoku                         */
/*  SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com) */
/*  https://github.com/sugoku/piuio-pico-brokeIO                              */
/******************************************************************************/

#ifndef _XINPUT_REPORT_H
#define _XINPUT_REPORT_H

#define XINPUT_ENDPOINT_SIZE 20

// Buttons 1 (8 bits)
// TODO: Consider using an enum class here.
#define XBOX_MASK_UP    (1U << 0)
#define XBOX_MASK_DOWN  (1U << 1)
#define XBOX_MASK_LEFT  (1U << 2)
#define XBOX_MASK_RIGHT (1U << 3)
#define XBOX_MASK_START (1U << 4)
#define XBOX_MASK_BACK  (1U << 5)
#define XBOX_MASK_LS    (1U << 6)
#define XBOX_MASK_RS    (1U << 7)

// Buttons 2 (8 bits)
// TODO: Consider using an enum class here.
#define XBOX_MASK_LB    (1U << 0)
#define XBOX_MASK_RB    (1U << 1)
#define XBOX_MASK_HOME  (1U << 2)
//#define UNUSED        (1U << 3)
#define XBOX_MASK_A     (1U << 4)
#define XBOX_MASK_B     (1U << 5)
#define XBOX_MASK_X     (1U << 6)
#define XBOX_MASK_Y     (1U << 7)

typedef struct __attribute((packed, aligned(1)))
{
	uint8_t report_id;
	uint8_t report_size;
	uint8_t buttons1;
	uint8_t buttons2;
	uint8_t lt;
	uint8_t rt;
	int16_t lx;
	int16_t ly;
	int16_t rx;
	int16_t ry;
	uint8_t _reserved[6];
} XInputReport;

static XInputReport xinputReport = {
	.report_id = 0,
	.report_size = XINPUT_ENDPOINT_SIZE,
	.buttons1 = 0,
	.buttons2 = 0,
	.lt = 0,
	.rt = 0,
	.lx = HID_JOYSTICK_MID,
	.ly = HID_JOYSTICK_MID,
	.rx = HID_JOYSTICK_MID,
	.ry = HID_JOYSTICK_MID,
	._reserved = { },
};

uint16_t xinput_get_report(XInputReport** report, struct inputArray* input) {
	xinputReport.buttons1 = 0
		| (!input->p1_cn  ? XBOX_MASK_UP    : 0)
		| (!input->p1_dr  ? XBOX_MASK_DOWN  : 0)
		| (!input->p1_dl  ? XBOX_MASK_LEFT  : 0)
		| (!input->p1_ur  ? XBOX_MASK_RIGHT : 0)
		| (!input->test    ? XBOX_MASK_START : 0)
		| (!input->service ? XBOX_MASK_BACK  : 0)
//		| (pressedL3()    ? XBOX_MASK_LS    : 0)
//		| (pressedR3()    ? XBOX_MASK_RS    : 0)
	;

	xinputReport.buttons2 = 0
		| (!input->p1_ul ? XBOX_MASK_LB   : 0)
		| (!input->p2_ur ? XBOX_MASK_RB   : 0)
		| (!input->clear ? XBOX_MASK_HOME : 0)
		| (!input->p2_dl ? XBOX_MASK_A    : 0)
		| (!input->p2_dr ? XBOX_MASK_B    : 0)
		| (!input->p2_ul ? XBOX_MASK_X    : 0)
		| (!input->p2_cn ? XBOX_MASK_Y    : 0)
	;

	/*
		use the following configuration for PIU Exceed on Xbox (thanks Crafty)
		| (!input->p2_cn  ? XBOX_MASK_RB    : 0)
		| (!input->p2_dr  ? XBOX_MASK_B  : 0)
		| (!input->p2_dl  ? XBOX_MASK_A  : 0)
		| (!input->p2_ur  ? XBOX_MASK_Y : 0)
		| (!input->p2_ul  ? XBOX_MASK_X : 0)
	*/
	

	//xinputReport.lx = static_cast<int16_t>(state.lx) + INT16_MIN;
	//xinputReport.ly = static_cast<int16_t>(~state.ly) + INT16_MIN;
	//xinputReport.rx = static_cast<int16_t>(state.rx) + INT16_MIN;
	//xinputReport.ry = static_cast<int16_t>(~state.ry) + INT16_MIN;

	xinputReport.lt = (!input->p1_coin) ? 0xFF : 0;
	xinputReport.rt = (!input->p2_coin) ? 0xFF : 0;

	*report = &xinputReport;
	return sizeof(XInputReport);
}

#endif