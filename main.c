
// check with racer to make sure we can submit this as MIT instead of GPL

#include "bsp/board.h"
#include "device/usbd.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "piuio_structs.h"
#include "piuio_config.h"

#include "input_mux4067.h"
#include "lights_latch32.h"

#ifdef ENABLE_WS2812_SUPPORT
#include "piuio_ws2812.h"
#endif

// const uint8_t pos[] = { 3, 0, 2, 1, 4 }; // don't touch this
// i touched it

// PIUIO input and output data
struct inputArray input = {
    .p1_ul = 1,
    .p1_ur = 1,
    .p1_cn = 1,
    .p1_dl = 1,
    .p1_dr = 1,
    .empty1 = 0b111,

    .empty2 = 0b1,
    .test = 1,
    .p1_coin = 1,
    .empty3 = 0b111,
    .service = 1,
    .clear = 1,

    .p2_ul = 1,
    .p2_ur = 1,
    .p2_cn = 1,
    .p2_dl = 1,
    .p2_dr = 1,
    .empty4 = 0b111,

    .empty5 = 0b11,
    .p2_coin = 1,
    .empty6 = 0b11111,
    .empty7 = 0xFFFFFFFF
};
struct lightsArray lights = {
    .p1_mux = 0,
    .p1_ul_light = 0,
    .p1_ur_light = 0,
    .p1_cn_light = 0,
    .p1_dl_light = 0,
    .p1_dr_light = 0,
    .empty1 = 0b1,
    
    .empty2 = 0b11,
    .bass_light = 0,
    .empty3 = 0b11111,
    
    .p2_mux = 0,
    .p2_ul_light = 0,
    .p2_ur_light = 0,
    .p2_cn_light = 0,
    .p2_dl_light = 0,
    .p2_dr_light = 0,
    .r2_halo = 0,
    
    .r1_halo = 0,
    .l2_halo = 0,
    .l1_halo = 0,
    .empty5 = 0b111,
    .r1_halo_dupe = 0,
    .r2_halo_dupe = 0,
    .empty6 = 0xFFFFFFFF
};

extern uint32_t mux4067_vals[5];

void update_input_mux() {
    uint32_t buf_mux_global = mux4067_vals[MUX_GLOBAL];
    uint32_t buf_mux_p1 = mux4067_vals[lights.p1_mux];
    uint32_t buf_mux_p2 = mux4067_vals[lights.p2_mux];

    // logic is negative
    input.p1_ul = !GETBIT(buf_mux_p1, MUX4067_P1_UPLEFT);
    input.p1_ur = !GETBIT(buf_mux_p1, MUX4067_P1_UPRIGHT);
    input.p1_cn = !GETBIT(buf_mux_p1, MUX4067_P1_CENTER);
    input.p1_dl = !GETBIT(buf_mux_p1, MUX4067_P1_DOWNLEFT);
    input.p1_dr = !GETBIT(buf_mux_p1, MUX4067_P1_DOWNRIGHT);

    input.p2_ul = !GETBIT(buf_mux_p2, MUX4067_P2_UPLEFT);
    input.p2_ur = !GETBIT(buf_mux_p2, MUX4067_P2_UPRIGHT);
    input.p2_cn = !GETBIT(buf_mux_p2, MUX4067_P2_CENTER);
    input.p2_dl = !GETBIT(buf_mux_p2, MUX4067_P2_DOWNLEFT);
    input.p2_dr = !GETBIT(buf_mux_p2, MUX4067_P2_DOWNRIGHT);

    input.p1_coin = !GETBIT(buf_mux_global, MUX4067_P1_COIN);
    input.p2_coin = !GETBIT(buf_mux_global, MUX4067_P2_COIN);

    input.test = !GETBIT(buf_mux_global, MUX4067_TEST);
    input.service = !GETBIT(buf_mux_global, MUX4067_SERVICE);
    input.clear = !GETBIT(buf_mux_global, MUX4067_CLEAR);
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) {
    // nothing to with DATA & ACK stage
    if (stage != CONTROL_STAGE_SETUP) return true;

    // Request 0xAE = IO Time
    if (request->bRequest == 0xAE) {
        switch (request->bmRequestType) {
            case 0x40:
                return tud_control_xfer(rhport, request, (void *)&lights.data, 8);
            case 0xC0:
                // just for now, in the future ideally the I/O polls the muxes itself
                // and should have the correct array immediately ready
                update_input_mux();
                
                return tud_control_xfer(rhport, request, (void *)&input.data, 8);
            default:
                return false;
        }
    }

    return false;
}

void input_task() {
    mux4067_update(lights.p1_mux, lights.p2_mux);

    // // P1 / P2 inputs
    // for (int i = 0; i < 5; i++) {
    //     uint8_t* p1 = &inputData[PLAYER_1];
    //     uint8_t* p2 = &inputData[PLAYER_2];
    //     *p1 = gpio_get(pinSwitch[i]) ? tu_bit_set(*p1, pos[i]) : tu_bit_clear(*p1, pos[i]);
    //     *p2 = gpio_get(pinSwitch[i+5]) ? tu_bit_set(*p2, pos[i]) : tu_bit_clear(*p2, pos[i]);
    // }

    // // Test/Service buttons
    // inputData[CABINET] = gpio_get(pinSwitch[10]) ? tu_bit_set(inputData[1], 1) : tu_bit_clear(inputData[1], 1);
    // inputData[CABINET] = gpio_get(pinSwitch[11]) ? tu_bit_set(inputData[1], 6) : tu_bit_clear(inputData[1], 6);
}

void lights_task() {
    #ifdef ENABLE_WS2812_SUPPORT
    ws2812_lock_mtx();
    #endif

    uint32_t buf;

    SETORCLRBIT(buf, LATCH_P1L_UPLEFT, lights.p1_ul_light);
    SETORCLRBIT(buf, LATCH_P1L_UPRIGHT, lights.p1_ur_light);
    SETORCLRBIT(buf, LATCH_P1L_CENTER, lights.p1_cn_light);
    SETORCLRBIT(buf, LATCH_P1L_DOWNLEFT, lights.p1_dl_light);
    SETORCLRBIT(buf, LATCH_P1L_DOWNRIGHT, lights.p1_dr_light);

    SETORCLRBIT(buf, LATCH_P2L_UPLEFT, lights.p2_ul_light);
    SETORCLRBIT(buf, LATCH_P2L_UPRIGHT, lights.p2_ur_light);
    SETORCLRBIT(buf, LATCH_P2L_CENTER, lights.p2_cn_light);
    SETORCLRBIT(buf, LATCH_P2L_DOWNLEFT, lights.p2_dl_light);
    SETORCLRBIT(buf, LATCH_P2L_DOWNRIGHT, lights.p2_dr_light);

    SETORCLRBIT(buf, LATCH_P1_S0, lights.p1_mux & 0b1);
    SETORCLRBIT(buf, LATCH_P1_S1, lights.p1_mux & 0b10);
    SETORCLRBIT(buf, LATCH_P2_S0, lights.p2_mux & 0b1);
    SETORCLRBIT(buf, LATCH_P2_S1, lights.p2_mux & 0b10);

    SETORCLRBIT(buf, LATCH_CABL_MARQ1, lights.l1_halo);
    SETORCLRBIT(buf, LATCH_CABL_MARQ2, lights.l2_halo);
    SETORCLRBIT(buf, LATCH_CABL_MARQ3, lights.r1_halo);
    SETORCLRBIT(buf, LATCH_CABL_MARQ4, lights.r2_halo);
    SETORCLRBIT(buf, LATCH_CABL_NEON, lights.bass_light);

    SETBIT(buf, LATCH_ALWAYS_ON);
    CLRBIT(buf, LATCH_COIN_COUNTER);

    lights_send(&buf);

    #ifdef ENABLE_WS2812_SUPPORT
    ws2812_unlock_mtx();
    #endif

    // // Write pad lamps
    // for (int i = 0; i < 5; i++) {
    //     gpio_put(pinLED[i], tu_bit_test(lamp.data[PLAYER_1], pos[i] + 2));
    //     gpio_put(pinLED[i+5], tu_bit_test(lamp.data[PLAYER_2], pos[i] + 2));
    // }

    // // Write the bass neon to the onboard LED for testing + kicks
    // gpio_put(25, lamp.bass_light);
}

int main() {
    board_init();

    // Init WS2812B
    #ifdef ENABLE_WS2812_SUPPORT
    ws2812_init(&lights);
    #endif

    mux4067_init();
    lights_init();

    // init device stack on configured roothub port
    tud_init(BOARD_TUD_RHPORT);

    // Main loop
    while (true) {
        tud_task(); // tinyusb device task

        lights_task();  // update mux before reading in input
        input_task();
    }

    return 0;
}
