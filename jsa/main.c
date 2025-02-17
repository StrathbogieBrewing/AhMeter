#include <string.h>

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "asm.h"

#include "buffer.h"
#include "tinbus.h"

#define RX_IS_RELEASED (PIND & (1 << PORTD0))

#define TX_ACTIVE (PORTD |= (1 << PORTD2))
#define TX_RELEASE (PORTD &= ~(1 << PORTD2))
#define TX_IS_ACTIVE (PORTD & (1 << PORTD2))

#define DATA_LENGTH 3
#define MESSAGE_SIZE (DATA_SIZE + 6)

#define MESSAGE_SHUNT_BASE 0x0800

#define ADC_SAMPLES 7
#define BIT_PERIOD 88 // 9600 bps with clock at 921.6 kHz (96 less 8 cycles)
#define BIT_PULSE 30  // pulse is 5/16 of bit period

uint8_t getSwitch(void) {
    return (0x0F & (((uint8_t)(~PIND)) >> 3)); // read address from rotary switch
}

// waits one bit period and returns true if bus is inactive
static bool bit_sync(void) {
    bool rx_inactive = true;
    while ((TIFR & (1 << OCF0A)) == 0) {
        if (!RX_IS_RELEASED) {
            rx_inactive = false;
        }
    }
    TCNT0 = 0;
    OCR0A = BIT_PERIOD;
    TIFR |= (1 << OCF0A);
    return rx_inactive;
}

// returns true if successful
bool tx_zero(void) {
    if (RX_IS_RELEASED || TX_IS_ACTIVE) {
        TX_ACTIVE;
        return !bit_sync();
    }
    return false;
}

// returns true if successful
bool tx_one(void) {
    TX_RELEASE;
    _delay_us(10);
    return bit_sync();
}

bool send(uint8_t data[], uint8_t bytes_to_send) {
    for (uint8_t i = 0; i < 20; i++) { // minimum 15 bit periods between frames
        if (!tx_one()) {
            return false;
        }
    }
    while (bytes_to_send--) {
        uint8_t mask = 0x80; // send bits in canbus order (msb first)
        uint8_t byte = *data++;
        if (!tx_zero()) {
            return false;
        }
        while (mask != 0) {
            if (mask & byte) {
                if (!tx_one()) {
                    return false;
                }
            } else {
                if (!tx_zero()) {
                    return false;
                }
            }
            mask >>= 1;
        }
        if (!tx_one()) {
            return false;
        }
    }
    return true;
}

int main(void) {
    static int32_t current = 0;
    static uint8_t count = 0;

    wdt_enable(WDTO_250MS);
    asm_init();

    DDRD |= (1 << PORTD1); // init bus outputs
    DDRD |= (1 << PORTD2);

    DDRB |= (1 << PORTB2); // debug

    while (1) {
        wdt_reset();
        if (asm_ADC_doConversion()) // we now have about 150 ms before next adc interrupt
        {
            int32_t value = asm_ADC_getRawResult();
            current += value;
            count++;

            if (count >= ADC_SAMPLES) {
                count = 0;

                uint16_t id = MESSAGE_SHUNT_BASE | (uint16_t)getSwitch();

                buffer_t *data_buffer = BUFFER(DATA_LENGTH);
 
                data_buffer->data[0] = (uint32_t)current >> 16;
                data_buffer->data[1] = (uint32_t)current >> 8;
                data_buffer->data[2] = (uint32_t)current;
                current = 0;

                data_buffer->length = DATA_LENGTH;

                buffer_t *tx_buffer = BUFFER(TINBUS_FRAME_LENGTH(DATA_LENGTH));

                tinbus_build_frame(tx_buffer, id, data_buffer);

                uint8_t tries = 0;
                uint8_t priority = 3;
                while (tries < 5) {
                    if (send(tx_buffer->data, tx_buffer->length)) {
                        break;
                    }
                    tries++;
                    uint8_t i = ((priority & 0x07) << 2) + (TCNT0 & 0x03);
                    while (i--) {
                        _delay_us(50); // approx 1/2 bit periods
                    }
                }
            }
        }
    }
    return 0;
}
