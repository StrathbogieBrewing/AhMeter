#include <string.h>

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "asm.h"
#include "crc8.h"

#define RX_IS_RELEASED (PIND & (1 << PORTD0))
#define TX_ACTIVE (PORTD |= (1 << PORTD2))
#define TX_RELEASE (PORTD &= ~(1 << PORTD2))
#define TX_IS_ACTIVE (PORTD & (1 << PORTD2))

#define MESSAGE_SIZE 4
#define FRAME_SIZE (MESSAGE_SIZE + 1) // add a byte for the crc

#define MESSAGE_SHUNT_BASE 0x80

#define ADC_SAMPLES 7
#define BIT_PERIOD_US 88 // 9600 bps with clock at 921.6 kHz (96 less 8 cycles)
#define BIT_PULSE_US 20  // pulse is approx 3/16 of bit period

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
    OCR0A = BIT_PERIOD_US;
    TIFR |= (1 << OCF0A);
    return rx_inactive;
}

// returns true if successful
bool tx_zero(void) {
    if (RX_IS_RELEASED) {
        TX_ACTIVE;
        _delay_us(BIT_PULSE_US);
        TX_RELEASE;
        _delay_us(BIT_PULSE_US / 2);
        return bit_sync();
    }
    return false;
}

// returns true if successful
bool tx_one(void) {
    return bit_sync();
}

bool send(uint8_t data[], uint8_t bytes_to_send) {
    for (uint8_t i = 0; i < 20; i++) { // minimum 20 bit periods between bus activity
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

                uint8_t frame[FRAME_SIZE];
                // set lsb to zero to allow for future 16 bit ids
                uint8_t id = MESSAGE_SHUNT_BASE | (getSwitch() << 1);
                frame[0] = id;
                frame[1] = (uint32_t)current >> 16;
                frame[2] = (uint32_t)current >> 8;
                frame[3] = (uint32_t)current;
                current = 0;
                uint8_t crc = 0;
                for (uint8_t index = 0; index < MESSAGE_SIZE; index++) {
                    crc = crc8(crc, frame[index]);
                }
                frame[4] = crc;

                uint8_t tries = 0;
                uint8_t priority = 3;
                while (tries < 5) {
                    if (send(frame, FRAME_SIZE)) {
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
