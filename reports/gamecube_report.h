/******************************************************************************/
/* SPDX-License-Identifier: MIT                                               */
/* SPDX-FileCopyrightText: Copyright (c) 2023 sugoku                          */
/* SPDX-FileCopyrightText: Copyright (c) 2022 arpruss                         */
/* SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)  */
/* https://github.com/sugoku/piuio-pico-brokeIO                               */
/******************************************************************************/

#ifndef _GAMECUBE_REPORT_H
#define _GAMECUBE_REPORT_H

#include "input_mux4067.h"

extern uint32_t mux4067_vals_db[MUX_COUNT];

#define GAMECUBE_CONTROLLER_ENABLED 0x14 // NORMAL, POWERED
//#define GAMECUBE_CONTROLLER_ENABLED 0x10 // NORMAL (0x22=WAVEBIRD COMMUNICATING)

#define GAMECUBE_CONTROLLER_DISABLED 0x0

#define GAMECUBE_MAIN_REPORT_ID 0x21
#define GAMECUBE_RUMBLE_REPORT_ID 17

#define GAMECUBE_ENDPOINT_SIZE 64

// Button report (16 bits)
#define GAMECUBE_MASK_A       (1U <<  0)
#define GAMECUBE_MASK_B       (1U <<  1)
#define GAMECUBE_MASK_X       (1U <<  2)
#define GAMECUBE_MASK_Y       (1U <<  3)
#define GAMECUBE_MASK_DLEFT       (1U <<  4)
#define GAMECUBE_MASK_DRIGHT      (1U <<  5)
#define GAMECUBE_MASK_DDOWN      (1U <<  6)
#define GAMECUBE_MASK_DUP   (1U <<  7)
#define GAMECUBE_MASK_START (1U <<  8)
#define GAMECUBE_MASK_Z    (1U <<  9)
#define GAMECUBE_MASK_R      (1U << 10)
#define GAMECUBE_MASK_L      (1U << 11)

// GameCube analog sticks only report 8 bits
#define GAMECUBE_JOYSTICK_MIN 0x00
#define GAMECUBE_JOYSTICK_MID 0x80
#define GAMECUBE_JOYSTICK_MAX 0xFF

typedef struct __attribute((packed, aligned(1))) {
  uint16_t buttons;
  uint8_t joystickX;
  uint8_t joystickY;
  uint8_t cX;
  uint8_t cY;
  uint8_t shoulderLeft;
  uint8_t shoulderRight;
} GameCubeData_t;

typedef struct __attribute((packed, aligned(1))) {
  uint8_t state;
  GameCubeData_t data;
} GameCubeSingleController_t;

typedef struct __attribute((packed, aligned(1))) {
  uint8_t reportId;
  GameCubeSingleController_t payload[4];
} GameCubeReport;

static_assert(sizeof(GameCubeReport)==37, "Wrong packing!");

static GameCubeReport gamecubeReport = {
    .reportId = GAMECUBE_MAIN_REPORT_ID,

    .payload[0].state = 0,
    .payload[0].data.buttons = 0,
    .payload[0].data.joystickX = GAMECUBE_JOYSTICK_MID,
    .payload[0].data.joystickY = GAMECUBE_JOYSTICK_MID,
    .payload[0].data.cX = GAMECUBE_JOYSTICK_MID,
    .payload[0].data.cY = GAMECUBE_JOYSTICK_MID,
    .payload[0].data.shoulderLeft = GAMECUBE_JOYSTICK_MIN,
    .payload[0].data.shoulderRight = GAMECUBE_JOYSTICK_MIN,

    .payload[1].state = 0,
    .payload[1].data.buttons = 0,
    .payload[1].data.joystickX = GAMECUBE_JOYSTICK_MID,
    .payload[1].data.joystickY = GAMECUBE_JOYSTICK_MID,
    .payload[1].data.cX = GAMECUBE_JOYSTICK_MID,
    .payload[1].data.cY = GAMECUBE_JOYSTICK_MID,
    .payload[1].data.shoulderLeft = GAMECUBE_JOYSTICK_MIN,
    .payload[1].data.shoulderRight = GAMECUBE_JOYSTICK_MIN,

    .payload[2].state = 0,
    .payload[2].data.buttons = 0,
    .payload[2].data.joystickX = GAMECUBE_JOYSTICK_MID,
    .payload[2].data.joystickY = GAMECUBE_JOYSTICK_MID,
    .payload[2].data.cX = GAMECUBE_JOYSTICK_MID,
    .payload[2].data.cY = GAMECUBE_JOYSTICK_MID,
    .payload[2].data.shoulderLeft = GAMECUBE_JOYSTICK_MIN,
    .payload[2].data.shoulderRight = GAMECUBE_JOYSTICK_MIN,

    .payload[3].state = 0,
    .payload[3].data.buttons = 0,
    .payload[3].data.joystickX = GAMECUBE_JOYSTICK_MID,
    .payload[3].data.joystickY = GAMECUBE_JOYSTICK_MID,
    .payload[3].data.cX = GAMECUBE_JOYSTICK_MID,
    .payload[3].data.cY = GAMECUBE_JOYSTICK_MID,
    .payload[3].data.shoulderLeft = GAMECUBE_JOYSTICK_MIN,
    .payload[3].data.shoulderRight = GAMECUBE_JOYSTICK_MIN
};

// configured for 2-player Mario Kart 8 Deluxe
// for each controller, we have DL as stick left, DR as stick right, CN as R (drift), UL as L + stick down (throw item backwards + menu down), UR as R + stick up (throw item forwards + menu up)
// ideally on button board we have yellow left for down on d-pad, yellow right for up on d-pad, green select for A button, red for B button
uint16_t gamecube_get_report(GameCubeReport** report, struct inputArray* input, struct inputArray* last_input, bool player_toggle, bool jamma_w, bool jamma_x, bool jamma_y, bool jamma_z) {
    gamecubeReport.payload[0].state = GAMECUBE_CONTROLLER_ENABLED;
    gamecubeReport.payload[1].state = GAMECUBE_CONTROLLER_ENABLED;

    // when both inputs are held, the most recent input is used.
    if (!input->p1_dl && !input->p1_dr) {
        if (!last_input->p1_dl && last_input->p1_dr) {
            gamecubeReport.payload[0].data.joystickX = GAMECUBE_JOYSTICK_MAX;
        } else if (last_input->p1_dl && !last_input->p1_dr) {
            gamecubeReport.payload[0].data.joystickX = GAMECUBE_JOYSTICK_MIN;
        } else {
            gamecubeReport.payload[0].data.joystickX = GAMECUBE_JOYSTICK_MID;
        }
    } else if (!input->p1_dl) {
        gamecubeReport.payload[0].data.joystickX = GAMECUBE_JOYSTICK_MIN;
    } else if (!input->p1_dr) {
        gamecubeReport.payload[0].data.joystickX = GAMECUBE_JOYSTICK_MAX;
    } else {
        gamecubeReport.payload[0].data.joystickX = GAMECUBE_JOYSTICK_MID;
    }

    if (!input->p2_dl && !input->p2_dr) {
        if (!last_input->p2_dl && last_input->p2_dr) {
            gamecubeReport.payload[1].data.joystickX = GAMECUBE_JOYSTICK_MAX;
        } else if (last_input->p2_dl && !last_input->p2_dr) {
            gamecubeReport.payload[1].data.joystickX = GAMECUBE_JOYSTICK_MIN;
        } else {
            gamecubeReport.payload[1].data.joystickX = GAMECUBE_JOYSTICK_MID;
        }
    } else if (!input->p2_dl) {
        gamecubeReport.payload[1].data.joystickX = GAMECUBE_JOYSTICK_MIN;
    } else if (!input->p2_dr) {
        gamecubeReport.payload[1].data.joystickX = GAMECUBE_JOYSTICK_MAX;
    } else {
        gamecubeReport.payload[1].data.joystickX = GAMECUBE_JOYSTICK_MID;
    }

    #define MARIO_KART

    #if defined(MARIO_KART)
    // note that Y is flipped
    gamecubeReport.payload[0].data.joystickY = !input->p1_ul ? GAMECUBE_JOYSTICK_MAX : GAMECUBE_JOYSTICK_MID;
    gamecubeReport.payload[0].data.joystickY = !input->p1_ur ? GAMECUBE_JOYSTICK_MIN : gamecubeReport.payload[0].data.joystickY;
    // if (!input->test)
    //     gamecubeReport.payload[0].data.joystickY = GAMECUBE_JOYSTICK_MAX;

    gamecubeReport.payload[1].data.joystickY = !input->p2_ul ? GAMECUBE_JOYSTICK_MAX : GAMECUBE_JOYSTICK_MID;
    gamecubeReport.payload[1].data.joystickY = !input->p2_ur ? GAMECUBE_JOYSTICK_MIN : gamecubeReport.payload[1].data.joystickY;
    // if (!input->service)
    //     gamecubeReport.payload[1].data.joystickY = GAMECUBE_JOYSTICK_MAX;

    gamecubeReport.payload[0].data.buttons = 0
        | (!input->clear ? GAMECUBE_MASK_START  : 0)
        | (!input->p1_coin ? GAMECUBE_MASK_START  : 0)
        | (!input->test ? GAMECUBE_MASK_A       : 0)
        | (!input->p1_ul ? GAMECUBE_MASK_L       : 0)
        | (!input->p1_ur ? GAMECUBE_MASK_L       : 0)
        | (!input->p1_cn ? GAMECUBE_MASK_R       : 0)
    ;

    gamecubeReport.payload[1].data.buttons = 0
        | (!input->clear ? GAMECUBE_MASK_START  : 0)
        | (!input->p2_coin ? GAMECUBE_MASK_START  : 0)
        | (!input->service ? GAMECUBE_MASK_A       : 0)
        | (!input->p2_ul ? GAMECUBE_MASK_L       : 0)
        | (!input->p2_ur ? GAMECUBE_MASK_L       : 0)
        | (!input->p2_cn ? GAMECUBE_MASK_R       : 0)
    ;

    gamecubeReport.payload[player_toggle].data.buttons |= 
        //(GETBIT(mux4067_vals_db[MUX_GLOBAL], MUX4067_JAMMA_17) ? GAMECUBE_MASK_A   : 0)
        (jamma_w ? GAMECUBE_MASK_A   : 0)
        | (jamma_x ? GAMECUBE_MASK_B   : 0)
        | (jamma_y ? GAMECUBE_MASK_X   : 0)
        | (jamma_z ? GAMECUBE_MASK_START   : 0)
    ;
    #elif defined(PUYO_PUYO_TETRIS)
    // note that Y is flipped
    gamecubeReport.payload[0].data.joystickY = !input->p1_ul ? GAMECUBE_JOYSTICK_MAX : GAMECUBE_JOYSTICK_MID;
    gamecubeReport.payload[0].data.joystickY = !input->p1_ur ? GAMECUBE_JOYSTICK_MIN : gamecubeReport.payload[0].data.joystickY;
    // if (!input->test)
    //     gamecubeReport.payload[0].data.joystickY = GAMECUBE_JOYSTICK_MAX;

    gamecubeReport.payload[1].data.joystickY = !input->p2_ul ? GAMECUBE_JOYSTICK_MAX : GAMECUBE_JOYSTICK_MID;
    gamecubeReport.payload[1].data.joystickY = !input->p2_ur ? GAMECUBE_JOYSTICK_MIN : gamecubeReport.payload[1].data.joystickY;
    // if (!input->service)
    //     gamecubeReport.payload[1].data.joystickY = GAMECUBE_JOYSTICK_MAX;
    
    gamecubeReport.payload[0].data.buttons = 0
        | (!input->clear ? GAMECUBE_MASK_START  : 0)
        | (!input->p1_coin ? GAMECUBE_MASK_START  : 0)
        | (!input->test ? GAMECUBE_MASK_B       : 0)
        | (!input->p1_ul ? GAMECUBE_MASK_L       : 0)
        | (!input->p1_cn ? GAMECUBE_MASK_A       : 0)
    ;

    gamecubeReport.payload[1].data.buttons = 0
        | (!input->clear ? GAMECUBE_MASK_START  : 0)
        | (!input->p1_coin ? GAMECUBE_MASK_START  : 0)
        | (!input->test ? GAMECUBE_MASK_B       : 0)
        | (!input->p1_ul ? GAMECUBE_MASK_L       : 0)
        | (!input->p1_cn ? GAMECUBE_MASK_A       : 0)
    ;

    gamecubeReport.payload[player_toggle].data.buttons |= 
        //(GETBIT(mux4067_vals_db[MUX_GLOBAL], MUX4067_JAMMA_17) ? GAMECUBE_MASK_A   : 0)
        (jamma_w ? GAMECUBE_MASK_A   : 0)
        | (jamma_x ? GAMECUBE_MASK_B   : 0)
        | (jamma_y ? GAMECUBE_MASK_X   : 0)
        | (jamma_z ? GAMECUBE_MASK_START   : 0)
    ;
    #elif defined(SMASH_BROS)
    // note that Y is flipped
    gamecubeReport.payload[0].data.joystickY = !input->p1_ul ? GAMECUBE_JOYSTICK_MAX : GAMECUBE_JOYSTICK_MID;
    gamecubeReport.payload[0].data.joystickY = !input->p1_ur ? GAMECUBE_JOYSTICK_MIN : gamecubeReport.payload[0].data.joystickY;
    // if (!input->test)
    //     gamecubeReport.payload[0].data.joystickY = GAMECUBE_JOYSTICK_MAX;

    gamecubeReport.payload[1].data.joystickY = !input->p2_ul ? GAMECUBE_JOYSTICK_MAX : GAMECUBE_JOYSTICK_MID;
    gamecubeReport.payload[1].data.joystickY = !input->p2_ur ? GAMECUBE_JOYSTICK_MIN : gamecubeReport.payload[1].data.joystickY;
    // if (!input->service)
    //     gamecubeReport.payload[1].data.joystickY = GAMECUBE_JOYSTICK_MAX;

    gamecubeReport.payload[0].data.buttons = 0
        | (!input->clear ? GAMECUBE_MASK_START  : 0)
        | (!input->p1_coin ? GAMECUBE_MASK_START  : 0)
        | (!input->test ? GAMECUBE_MASK_A       : 0)
        | (!input->p1_ul ? GAMECUBE_MASK_X       : 0)
        | (!input->p1_ur ? GAMECUBE_MASK_B       : 0)
        | (!input->p1_cn ? GAMECUBE_MASK_A       : 0)
    ;

    gamecubeReport.payload[1].data.buttons = 0
        | (!input->clear ? GAMECUBE_MASK_START  : 0)
        | (!input->p1_coin ? GAMECUBE_MASK_START  : 0)
        | (!input->test ? GAMECUBE_MASK_A       : 0)
        | (!input->p1_ul ? GAMECUBE_MASK_X       : 0)
        | (!input->p1_ur ? GAMECUBE_MASK_B       : 0)
        | (!input->p1_cn ? GAMECUBE_MASK_A       : 0)
    ;

    gamecubeReport.payload[player_toggle].data.buttons |= 
        //(GETBIT(mux4067_vals_db[MUX_GLOBAL], MUX4067_JAMMA_17) ? GAMECUBE_MASK_A   : 0)
        (jamma_w ? GAMECUBE_MASK_A   : 0)
        | (jamma_x ? GAMECUBE_MASK_B   : 0)
        | (jamma_y ? GAMECUBE_MASK_X   : 0)
        | (jamma_z ? GAMECUBE_MASK_START   : 0)
    ;
    #endif

    *report = &gamecubeReport;
    return sizeof(GameCubeReport);
}

#endif