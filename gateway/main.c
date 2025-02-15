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
#include "uart.h"
#include "uart0.h"

#define RX_BUFFER_SIZE 32
#define RX_TIMEOUT TIMER_US_TO_TICKS((10 * 2 * 1000000) / 9600)

int main(void) {
    wdt_enable(WDTO_250MS);

    stdout = uart1_device;
    stdin = uart1_device;

    uint8_t rx_buffer[RX_BUFFER_SIZE] = {0};
    uint8_t rx_byte_count = 0;

    DDRC |= (1 << PORTC3);

    while (1) {
        wdt_reset();
        int rx_byte = fgetc(uart0_device);
        if (rx_byte != EOF) {
            rx_buffer[rx_byte_count++] = rx_byte;
            if (rx_byte_count >= RX_BUFFER_SIZE - 1) {
                rx_byte_count = 0; // do not overflow rx buffer, allow for terminating zero
            }
        } else if (rx_byte_count) { // check for 20 bit period timeout
            uint32_t rx_ticks = uart_get_rx_ticks(uart0_device);
            uint32_t ticks = timer_get_ticks();
            if (rx_ticks + RX_TIMEOUT < ticks) {
                uint8_t tx_byte_count = cobsm_encode(rx_buffer, rx_byte_count);
                rx_byte_count = 0;
                // rx_buffer[tx_byte_count++] = 0; // append a zero and send
                fwrite(rx_buffer, tx_byte_count, 1, stdout);
            }
        }
    }
    return 0;
}

// cli();
// uint16_t capture = timer1_capture;
//         timer1_capture = 0;
//         bool timeout = timer1_timeout;
//         timer1_timeout = false;
//         sei();
//         if (capture) {
//             if (last_capture) {
//                 uint16_t delta = capture - last_capture;
//                 if (delta < (4 * BIT_PERIOD_TICKS)) {
//                     rx_data(RX_ZERO);

//                 } else if (delta < (8 * BIT_PERIOD_TICKS)) {
//                     rx_data(RX_ONE);
//                 } else {
//                     last_capture = 0;
//                 }
//             }
//             last_capture = capture;
//         }
//         if (timeout) {
//             last_capture = 0;
//             rx_data(RX_END);
//         }

// #define BIT_PERIOD_TICKS ((50UL * F_CPU) / 1000000UL)
// #define BUFFER_SIZE_MAX 32
// #define MASK 0x80
// #define CRC_ERROR_COUNT 0xF0
// #define FRAME_ERROR_COUNT 0xF1

// enum { RX_ZERO = 0, RX_ONE, RX_END };

// volatile static uint16_t timer1_capture = 0;
// volatile static bool timer1_timeout = false;

// ISR(TIMER1_CAPT_vect) {
//     if (timer1_capture) {
//         PORTC ^= (1 << PORTC3);
//     }
//     if (ICR1) {
//         timer1_capture = ICR1;
//     } else {
//         timer1_capture = 1; // zero is used as a sentinel
//     }
//     OCR1A = ICR1 + (8 * BIT_PERIOD_TICKS); // setup end of frame timeout
//     TIFR1 |= (1 << OCF1A);
//     TIMSK1 |= (1 << OCIE1A);
// }

// ISR(TIMER1_COMPA_vect) {
//     timer1_timeout = true;
//     TIMSK1 &= ~(1 << OCIE1A);
// }

// void rx_data(uint8_t data) {
//     static uint8_t mask = MASK;
//     static uint8_t buffer[BUFFER_SIZE_MAX];
//     static uint8_t index = 0;
//     static uint16_t crc = 0xFFFF;
//     static uint8_t crc_error_count = 0;
//     static uint8_t frame_error_count = 0;
//     if (data == RX_END) {
//         crc = mb_crc(crc, buffer[index]);
//         uint8_t size = index + 1;
//         if (mask != 0){
//             frame_error_count++;
//             crc = 0xFFFF;
//             buffer[0] = FRAME_ERROR_COUNT;  // send message with frame error
//             count to indicate crc error crc = mb_crc(crc, buffer[0]);
//             buffer[1] = frame_error_count;
//             crc = mb_crc(crc, buffer[1]);
//             buffer[2] = crc & 0xFF;
//             buffer[3] = crc >> 8;
//             size = 4;
//         } else if (crc) {
//             crc_error_count++;
//             crc = 0xFFFF;
//             buffer[0] = CRC_ERROR_COUNT;    // send message with crc error
//             count to indicate crc error crc = mb_crc(crc, buffer[0]);
//             buffer[1] = crc_error_count;
//             crc = mb_crc(crc, buffer[1]);
//             buffer[2] = crc & 0xFF;
//             buffer[3] = crc >> 8;
//             size = 4;
//         }
//         size = cobsm_encode(buffer, size);  // encoding removes all zeros
//         from data
//         // size = cobsm_decode(buffer, size);

//         uint8_t i = 0;
//         while (i < size) {
//             putc(buffer[i++], stdout);  // send all bytes in frame
//         }
//         putc(0x00, stdout);             // mark end of frame with an added
//         zero

//         index = 0; // reset receiver buffer
//         buffer[index] = 0;
//         mask = MASK;
//         crc = 0xFFFF;
//         return;
//     }
//     if (mask == 0) {
//         crc = mb_crc(crc, buffer[index]);
//         if (index < BUFFER_SIZE_MAX - 1) {
//             index++;
//         } else {
//             index = 0; // just wrap on overflow for now
//         }
//         mask = MASK;
//         buffer[index] = 0;
//     }
//     if (data == RX_ONE) {
//         buffer[index] |= mask;
//     }
//     mask >>= 1;
// }

// void init_timer1(void) {
//     TCCR1B = (1 << CS10) | (1 << ICNC1); // -ve edge, noise cancel enabled,
//     no prescale TIMSK1 = (1 << ICIE1);               // | (1 << TOIE1);
//     sei();
// }