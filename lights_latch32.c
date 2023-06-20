/**********************************************************/
/*  SPDX-License-Identifier: MIT                          */
/*  https://github.com/sugoku/piuio-pico-brokeIO          */
/**********************************************************/

#include "lights_latch32.h"

PIO pio;
uint sm;

void lights_init() {
    
    gpio_init(LATCH_ENABLE_PIN);
    gpio_init(LATCH_RST_PIN);
    gpio_init(LATCH_RCLK_PIN);

    // set pinModes for non-native SPI pins
    gpio_set_dir(LATCH_ENABLE_PIN, 1);
    gpio_set_dir(LATCH_RST_PIN, 1);
    gpio_set_dir(LATCH_RCLK_PIN, 1);

    #ifndef SOFTWARE_LATCH
        spi_init(spi1, 14000000);
        // SPI.beginTransaction(SPISettings(14000000, MSBFIRST, SPI_MODE0));
    #else
        pio = pio0;
        sm = 0;   
        uint offset = pio_add_program(pio, &sspi_out_program);  // only sending, no receiving
        sspi_out_init(pio, sm, offset, SOFTWARE_SPI_DIN_PIN, SOFTWARE_SPI_CLK_PIN, SERIAL_CLK_DIV);
    #endif

    gpio_put(LATCH_ENABLE_PIN, 0);
    gpio_put(LATCH_RST_PIN, 1);
    gpio_put(LATCH_RCLK_PIN, 1);
}

void lights_send(uint32_t* buf) {
    // tell latch to receive from SPI
    gpio_put(LATCH_RCLK_PIN, 0);
    // the RP2040 is going too fast for our 74HC595s, so we need to delay every time we send messages out
    busy_wait_us(5);
    // note that we are using busy_wait_us instead of sleep_us because sleep_us halts our USB activity

    // worst case 10-15ns needed according to datasheet, but i guess we need longer
    #ifndef SOFTWARE_LATCH
        spi_write_blocking(spi1, buf, 4);
    #else
        sspi_out_put(pio, sm, (uint8_t)((*buf & 0xFF000000) >> 24));
        busy_wait_us(1);
        sspi_out_put(pio, sm, (uint8_t)((*buf & 0x00FF0000) >> 16));
        busy_wait_us(1);
        sspi_out_put(pio, sm, (uint8_t)((*buf & 0x0000FF00) >> 8));
        busy_wait_us(1);
        sspi_out_put(pio, sm, (uint8_t)((*buf & 0x000000FF)));
    #endif

    busy_wait_us(WAIT_LIGHTS_LATCH32);

    // tell latch to update values with what we just sent
    gpio_put(LATCH_RCLK_PIN, 1);

    busy_wait_us(WAIT_LIGHTS_LATCH32);
}

void lights_enable() {
    gpio_put(LATCH_ENABLE_PIN, 0);
}

void lights_disable() {
    gpio_put(LATCH_ENABLE_PIN, 1);
}

void lights_reset() {
    gpio_put(LATCH_RST_PIN, 0);
    busy_wait_us(5);  // just in case the latch takes some time to notice and reset
    gpio_put(LATCH_RST_PIN, 1);
}