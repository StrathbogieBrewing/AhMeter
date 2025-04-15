// #include <stdbool.h>

// #include <avr/interrupt.h>
#include <avr/io.h>

#include "timer.h"
#include "tinbus.h"
#include "uart0.h"

#define TINBUS_BUFFER_SIZE 16
#define RX_TIMEOUT (BIT_PERIOD * 20) // 20 bit periods

#define RX_TIMEOUT_TICKS (1823UL / TIMER_US_PER_TICK) // 2 characters

#define RX_IS_RELEASED (PIND & (1 << PORTD0))
#define TX_ACTIVATE (PORTD |= (1 << PORTD4))
#define TX_RELEASE (PORTD &= ~(1 << PORTD4))
#define TX_IS_ACTIVE (PORTD & (1 << PORTD4))

// static volatile uint8_t tinbus_rx_head = 0;
// static volatile uint8_t tinbus_rx_tail = 0;
// static uint16_t tinbus_rx_data[TINBUS_BUFFER_SIZE] = {0};

// static volatile uint8_t tinbus_tx_head = 0;
// static volatile uint8_t tinbus_tx_tail = 0;
// static uint8_t tinbus_tx_data[TINBUS_BUFFER_SIZE] = {0};

// ISR(TIMER1_CAPT_vect) {
//     uint16_t count = ICR1;
//     if (count == TINBUS_TIMEOUT) { // save 0xFFFF as a timeout sentinel value
//         count = 0;
//     }
//     tinbus_rx_data[tinbus_rx_head] = count;
//     tinbus_rx_head = (tinbus_rx_head + 1) & (TINBUS_BUFFER_SIZE - 1);
//     OCR1B = count + RX_TIMEOUT;
//     TIFR1 |= (1 << OCF1B);
//     TIMSK1 |= (1 << OCIE1B);
//     // PORTC ^= (1 << PORTC3);
// }

// ISR(TIMER1_COMPB_vect) {
//     tinbus_rx_data[tinbus_rx_head] = TINBUS_TIMEOUT;
//     tinbus_rx_head = (tinbus_rx_head + 1) & (TINBUS_BUFFER_SIZE - 1);
//     TIMSK1 &= ~(1 << OCIE1B);
//     // PORTC ^= (1 << PORTC3);
// }

// ISR(TIMER1_COMPA_vect) {
//     OCR1A += BIT_PERIOD;
//     PORTC ^= (1 << PORTC3);
// }

void tinbus_init(void) {
    PORTD &= ~(1 << PORTD4); // disable tx drive
    DDRD |= (1 << PORTD4);

    PORTD &= ~(1 << PORTD1); // enable tx
    DDRD |= (1 << PORTD1);
    // TCCR1B = (1 << ICNC1) | (1 << CS11); // normal mode, divide clock by 8
    // TIMSK1 = (1 << ICIE1) | (1 << OCIE1A);
    // sei();
}

enum { TINBUS_START, TINBUS_DATA, TINBUS_ERROR };

tinbus_rx_t tinbus_decode(int16_t rx_data) {
    static uint8_t rx_count = 0;
    static uint8_t rx_byte = 0;

    tinbus_rx_t result = {.status = TINBUS_RX_NO_DATA};

    if (rx_data == EOF){
        if ((rx_count & 0x03) != 0){
            result.status = TINBUS_RX_ERROR;
        } else if((rx_count & ~0x03) != 0){
            result.status = TINBUS_RX_FRAME_READY;
        }
        rx_count = 0;
    } else {
        rx_byte <<= 2;
        switch (rx_data) {
        case 0x80:
            rx_byte |= 0;
            break;
        case 0xE0:
            rx_byte |= 1;
            break;
        case 0xF8:
            rx_byte |= 2;
            break;
        case 0xFE:
            rx_byte |= 3;
            break;
        default:
            result.status = TINBUS_RX_ERROR;
            return result;
            break;
        }

        rx_count += 1;
        if ((rx_count & 0x03) == 0) {
            result.data = rx_byte;
            result.status = TINBUS_RX_DATA_READY;
        }
    }
    return result;
}

tinbus_rx_t tinbus_rx(void) {
    tinbus_rx_t result = {.status = TINBUS_RX_NO_DATA};

    int16_t rx_data = fgetc(uart0_device);
    if (rx_data != EOF) {
        // result.data = rx_data;
        // result.status = TINBUS_RX_DATA_READY;
        result = tinbus_decode(rx_data);
    }

    if (timer_get_ticks() > uart_get_rx_ticks(uart0_device) + RX_TIMEOUT_TICKS) {
        result = tinbus_decode(EOF);
        // result.status = TINBUS_RX_FRAME_READY;
    }

    return result;
}

// tinbus_rx_t tinbus_decode(uint8_t bit_periods) {
//     tinbus_rx_t result = {.status = TINBUS_RX_NO_DATA};
//     static uint8_t index;
//     static uint8_t data;
//     static uint8_t state = TINBUS_START;

//     if (bit_periods == (uint8_t)TINBUS_TIMEOUT) {
//         if (state != TINBUS_START) {
//             result.data = data;
//             result.status = TINBUS_RX_FRAME_READY;
//         }
//         state = TINBUS_START;
//     } else {
//         if (state == TINBUS_START) {
//             state = TINBUS_DATA;
//             data = 0xFF;
//             index = 0;
//         } else if (state == TINBUS_DATA) {
//             if (bit_periods == 0) {
//                 state = TINBUS_ERROR;
//                 result.status = TINBUS_RX_ERROR;
//             } else if (bit_periods + index > 8) { // must be a start bit
//                 result.data = data;
//                 result.status = TINBUS_RX_DATA_READY;
//                 data = 0xFF;
//                 index = 0;
//             } else {
//                 index += bit_periods;
//                 data &= ~(0x80 >> (index - 1));
//             }
//         }
//     }
//     return result;
// }

// tinbus_rx_t tinbus_rx(void) {
//     static uint16_t last_count = 0;
//     tinbus_rx_t result = {.status = TINBUS_RX_NO_DATA};

//     while (tinbus_rx_head != tinbus_rx_tail) {
//         uint16_t count = tinbus_rx_data[tinbus_rx_tail];
//         tinbus_rx_tail = (tinbus_rx_tail + 1) & (TINBUS_BUFFER_SIZE - 1);
//         uint16_t delta = count;
//         if (delta != TINBUS_TIMEOUT) {
//             delta -= last_count;
//             last_count = count;
//             delta += BIT_PERIOD / 2;
//             delta /= BIT_PERIOD;
//         }
//         result = tinbus_decode((uint8_t)delta);
//         if (result.status != TINBUS_RX_NO_DATA) {
//             break;
//         }
//     }
//     return result;
// }

// bool tinbus_tx(void){

// }