#include <stdbool.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "tinbus.h"

#define TINBUS_TIMEOUT 0xFFFF
#define TINBUS_BUFFER_SIZE 16
#define BIT_PERIOD 48
#define RX_TIMEOUT (BIT_PERIOD * 20) // 20 bit periods

static volatile uint8_t tinbus_rx_head = 0;
static volatile uint8_t tinbus_rx_tail = 0;
static uint16_t tinbus_rx_data[TINBUS_BUFFER_SIZE] = {0};

// static volatile uint8_t tinbus_tx_head = 0;
// static volatile uint8_t tinbus_tx_tail = 0;
// static uint8_t tinbus_tx_data[TINBUS_BUFFER_SIZE] = {0};

ISR(TIMER1_CAPT_vect) {
    uint16_t count = ICR1;
    if (count == TINBUS_TIMEOUT) { // save 0xFFFF as a timeout sentinel value
        count = 0;
    }
    tinbus_rx_data[tinbus_rx_head] = count;
    tinbus_rx_head = (tinbus_rx_head + 1) & (TINBUS_BUFFER_SIZE - 1);
    OCR1B = count + RX_TIMEOUT;
    TIFR1 |= (1 << OCF1B);
    TIMSK1 |= (1 << OCIE1B);
    // PORTC ^= (1 << PORTC3);
}

ISR(TIMER1_COMPB_vect) {
    tinbus_rx_data[tinbus_rx_head] = TINBUS_TIMEOUT;
    tinbus_rx_head = (tinbus_rx_head + 1) & (TINBUS_BUFFER_SIZE - 1);
    TIMSK1 &= ~(1 << OCIE1B);
    // PORTC ^= (1 << PORTC3);
}

ISR(TIMER1_COMPA_vect) {
    OCR1A += BIT_PERIOD;
    PORTC ^= (1 << PORTC3);
}

void tinbus_init(void) {
    TCCR1B = (1 << ICNC1) | (1 << CS11); // normal mode, divide clock by 8
    TIMSK1 = (1 << ICIE1) | (1 << OCIE1A);
    sei();
}

enum { TINBUS_START, TINBUS_DATA, TINBUS_ERROR };

tinbus_rx_t tinbus_decode(uint8_t bit_periods) {
    tinbus_rx_t result = {.status = TINBUS_RX_NO_DATA};
    static uint8_t index;
    static uint8_t data;
    static uint8_t state = TINBUS_START;

    if (bit_periods == (uint8_t)TINBUS_TIMEOUT) {
        if (state != TINBUS_START) {
            result.data = data;
            result.status = TINBUS_RX_FRAME_READY;
        }
        state = TINBUS_START;
    } else {
        if (state == TINBUS_START) {
            state = TINBUS_DATA;
            data = 0xFF;
            index = 0;
        } else if (state == TINBUS_DATA) {
            if (bit_periods == 0) {
                state = TINBUS_ERROR;
                result.status = TINBUS_RX_ERROR;
            } else if (bit_periods + index > 8) { // must be a start bit
                result.data = data;
                result.status = TINBUS_RX_DATA_READY;
                data = 0xFF;
                index = 0;
            } else {
                index += bit_periods;
                data &= ~(0x80 >> (index - 1));
            }
        }
    }

    return result;
}

tinbus_rx_t tinbus_rx(void) {
    static uint16_t last_count = 0;
    tinbus_rx_t result = {.status = TINBUS_RX_NO_DATA};

    while (tinbus_rx_head != tinbus_rx_tail) {
        uint16_t count = tinbus_rx_data[tinbus_rx_tail];
        tinbus_rx_tail = (tinbus_rx_tail + 1) & (TINBUS_BUFFER_SIZE - 1);
        uint16_t delta = count;
        if (delta != TINBUS_TIMEOUT) {
            delta -= last_count;
            last_count = count;
            delta += BIT_PERIOD / 2;
            delta /= BIT_PERIOD;
        }
        result = tinbus_decode((uint8_t)delta);
        if (result.status != TINBUS_RX_NO_DATA) {
            break;
        }
    }
    return result;
}

// bool tinbus_tx(void){

// }