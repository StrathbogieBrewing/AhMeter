#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

// #include "cobsm.h"
#include "crc8.h"
#include "tinbus.h"
#include "uart1.h"

#define RX_BUFFER_SIZE 16
// #define RX_TIMEOUT TIMER_US_TO_TICKS((15 * 1000000) / 9600)

#define DEVICE_JSA (0x40)

bool decode(uint8_t buffer[], uint8_t size) {
    if (size == 0) {
        return false;
    }

    uint8_t device_type = buffer[0] & 0xF0;
    uint8_t device_id = buffer[0] & 0x0F;

    if (device_type == DEVICE_JSA) {
        if (size != 4) {
            return false;
        }

        int32_t value = ((uint32_t)buffer[1] << 24) + ((uint32_t)buffer[2] << 16) + ((uint32_t)buffer[3] << 8);

        switch (device_id) {
        case 0: // Battery Current
            fprintf(stdout, "{\"Batt_mA\":%ld}\n", value / 106);
            return true;
        case 1: // Load Current
            fprintf(stdout, "{\"Load_mA\":%ld}\n", value / 106);
            return true;
        case 2: // Dump Current
            fprintf(stdout, "{\"Dump_mA\":%ld}\n", value / 106);
            return true;
        case 4: // Battery 1 Voltage
            fprintf(stdout, "{\"Batt_1_mV\":%ld}\n", value / 1408);
            return true;
        case 5: // Battery 2 Voltage
            fprintf(stdout, "{\"Batt_2_mV\":%ld}\n", value / 1416);
            return true;
        default:
            return false;
        }
    }
    return false;
}

int main(void) {
    wdt_enable(WDTO_250MS);

    stdout = uart1_device;
    stdin = uart1_device;

    tinbus_init();

    uint8_t rx_buffer[RX_BUFFER_SIZE] = {0};
    uint8_t rx_byte_count = 0;
    uint8_t rx_crc = 0;
    uint32_t rx_crc_errors = 0;
    uint32_t rx_tinbus_errors = 0;
    uint32_t rx_overrun_errors = 0;
    uint32_t rx_device_errors = 0;

    DDRC |= (1 << PORTC3);

    while (1) {
        wdt_reset();
        tinbus_rx_t rx_data = tinbus_rx();
        if (rx_data.status == TINBUS_RX_DATA_READY) {
            rx_buffer[rx_byte_count++] = rx_data.data;
            rx_crc = crc8(rx_crc, rx_data.data);
            if (rx_byte_count >= RX_BUFFER_SIZE) {
                rx_overrun_errors += 1;
                fprintf(stdout, "{\"overrun_errors\":%ld}", rx_overrun_errors);
                rx_byte_count = 0;
                rx_crc = 0;
            }
        }
        if (rx_data.status == TINBUS_RX_ERROR) {
            rx_tinbus_errors += 1;
            fprintf(stdout, "{\"tinbus_errors\":%ld}", rx_tinbus_errors);
            rx_byte_count = 0;
            rx_crc = 0;
        }
        if (rx_data.status == TINBUS_RX_FRAME_READY) {
            if (rx_crc) {
                rx_crc_errors += 1;
                fprintf(stdout, "{\"crc_errors\":%ld}", rx_crc_errors);
            } else {
                if(rx_byte_count){
                    rx_byte_count -= 1;
                }
                if (decode(rx_buffer, rx_byte_count) == false) {
                    rx_device_errors += 1;
                    fprintf(stdout, "{\"device_errors\":%ld}", rx_device_errors);
                }
            }
            rx_byte_count = 0;
            rx_crc = 0;
        }
    }
    return 0;
}
