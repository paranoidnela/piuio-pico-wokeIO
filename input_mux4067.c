/**********************************************************/
/*  SPDX-License-Identifier: MIT                          */
/*  https://github.com/sugoku/piuio-pico-brokeIO          */
/**********************************************************/

#include "input_mux4067.h"

uint32_t mux4067_vals[5];
// uint8_t last_mux;

#define MUX_P1_INPUTS_SIZE 5
const uint8_t mux_p1_inputs[] = {
    MUX4067_P1_UPLEFT,
    MUX4067_P1_UPRIGHT,
    MUX4067_P1_CENTER,
    MUX4067_P1_DOWNLEFT,
    MUX4067_P1_DOWNRIGHT,
};
#define MUX_P2_INPUTS_SIZE 5
const uint8_t mux_p2_inputs[] = {
    MUX4067_P2_UPLEFT,
    MUX4067_P2_UPRIGHT,
    MUX4067_P2_CENTER,
    MUX4067_P2_DOWNLEFT,
    MUX4067_P2_DOWNRIGHT,
};

void mux4067_init() {
    // last_mux = 0;

    gpio_init(MUX_ENABLE_PIN);
    gpio_init(MUX1_IN_PIN);
    gpio_init(MUX2_IN_PIN);

    gpio_init(MUX_S0_PIN);
    gpio_init(MUX_S1_PIN);
    gpio_init(MUX_S2_PIN);
    gpio_init(MUX_S3_PIN);

    // set pinModes
    gpio_set_dir(MUX_ENABLE_PIN, true);  // set to output

    gpio_set_dir(MUX1_IN_PIN, false);  // set to input
    gpio_set_dir(MUX2_IN_PIN, false);  // set to input
    #ifdef PULLUP_IN
        gpio_pull_up(MUX1_IN_PIN);  // enable pull-up resistor
        gpio_pull_up(MUX2_IN_PIN);  // enable pull-up resistor
    #endif

    gpio_set_dir(MUX_S0_PIN, true);  // set to output
    gpio_set_dir(MUX_S1_PIN, true);  // set to output
    gpio_set_dir(MUX_S2_PIN, true);  // set to output
    gpio_set_dir(MUX_S3_PIN, true);  // set to output

    // make sure everything is low
    mux4067_reset();
    // enable MUXes
    mux4067_enable();
}

void mux4067_enable() {
    gpio_put(MUX_ENABLE_PIN, 0);
}

void mux4067_disable() {
    gpio_put(MUX_ENABLE_PIN, 1);
}

void mux4067_reset() {
    // set read inputs to 0
    for (int i = 0; i < 4; i++)
        mux4067_vals[i] = 0;

    // clear all selector pins
    gpio_put(MUX_S0_PIN, 0);
    gpio_put(MUX_S1_PIN, 0);
    gpio_put(MUX_S2_PIN, 0);
    gpio_put(MUX_S3_PIN, 0);

    // disable MUX until enabled again
    mux4067_disable();
}

void mux4067_update(uint8_t mux_p1, uint8_t mux_p2) {
    for (int i = 15; i >= 0; i--) {
        // set the selector pins to the current value
        gpio_put(MUX_S3_PIN, (i >> 3) & 1);
        gpio_put(MUX_S2_PIN, (i >> 2) & 1);
        gpio_put(MUX_S1_PIN, (i >> 1) & 1);
        gpio_put(MUX_S0_PIN, i & 1);

        busy_wait_us(5);  // wait for selector to change

        // select mux based on if the input is related to P1 or P2 mux or neither
        uint8_t mux1 = MUX_GLOBAL;
        uint8_t mux2 = MUX_GLOBAL;
        for (int j = 0; j < MUX_P1_INPUTS_SIZE; j++) {
            if (i == mux_p1_inputs[j]) {
                mux1 = mux_p1;
            } else if (i + 16 == mux_p1_inputs[j]) {
                mux2 = mux_p1;
            }
        }
        for (int j = 0; j < MUX_P2_INPUTS_SIZE; j++) {
            if (i == mux_p2_inputs[j]) {
                mux1 = mux_p2;
            } else if (i + 16 == mux_p2_inputs[j]) {
                mux2 = mux_p2;
            }
        }

        // read this value from BOTH muxes and store them in the ith bit and the (i+16)th bit respectively
        #ifdef PULLUP_IN
            SETORCLRBIT(mux4067_vals[mux1], i, !gpio_get(MUX1_IN_PIN));
            SETORCLRBIT(mux4067_vals[mux2], i+16, !gpio_get(MUX2_IN_PIN));
        #else
            SETORCLRBIT(mux4067_vals[mux1], i, gpio_get(MUX1_IN_PIN));
            SETORCLRBIT(mux4067_vals[mux2], i+16, gpio_get(MUX2_IN_PIN));
        #endif
    }
}

uint32_t mux4067_merged(uint32_t* vals) {
    return vals[0] | vals[1] | vals[2] | vals[3];
}