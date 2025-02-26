#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#define UART1_BAUD 9600UL
#include "uart1.h"

#include "cobsm.h"
#include "timer.h"
#include "tinbus.h"
#include "uart.h"
#include "uart0.h"

#define RX_BUFFER_SIZE 256
#define RX_TIMEOUT TIMER_US_TO_TICKS((15 * 1000000) / 9600)

int main(void) {
    wdt_enable(WDTO_250MS);

    stdout = uart1_device;
    stdin = uart1_device;

    tinbus_init();

    uint8_t rx_buffer[RX_BUFFER_SIZE] = {0};
    uint8_t rx_byte_count = 0;

    DDRC |= (1 << PORTC3);

    while (1) {
        wdt_reset();
        tinbus_rx_t rx_data = tinbus_rx();
        if (rx_data.status == TINBUS_RX_DATA_READY) {
            rx_buffer[rx_byte_count++] = rx_data.data;
        }
        if (rx_data.status == TINBUS_RX_ERROR) {
            rx_byte_count = 0;
        }
        if (rx_data.status == TINBUS_RX_FRAME_READY) {
            rx_buffer[rx_byte_count++] = rx_data.data;
            uint8_t tx_byte_count = cobsm_encode(rx_buffer, rx_byte_count);
            fwrite(rx_buffer, tx_byte_count, 1, stdout);
            rx_byte_count = 0;
        }
    }
    return 0;
}

