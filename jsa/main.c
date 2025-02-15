#include <string.h>

#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "asm.h"
#include "crc8.h"
#include "tinbus.h"

#define RX_IS_RELEASED (PIND & (1 << PORTD0))

#define TX_ACTIVE (PORTD |= (1 << PORTD2))
#define TX_RELEASE (PORTD &= ~(1 << PORTD2))
#define TX_IS_ACTIVE (PORTD & (1 << PORTD2))

#define DATA_SIZE 6
#define MESSAGE_SIZE (DATA_SIZE + 6)

#define MESSAGE_SHUNT_BASE 0x10

#define ADC_SAMPLES 4
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
    for (uint8_t i = 0; i < 30; i++) { // 3 character periods between frames
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
    static int64_t charge = 0;
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
            if ((value <= 1) && (value >= -1)) {
                value = 0; // remove zero offset error for ah accumulation
            }
            charge += (int64_t)value;
            count++;

            if (count >= ADC_SAMPLES) {
                count = 0;

                uint8_t id = getSwitch();
                uint32_t msg_id = get_tinbus_ext_id(id, TINBUS_PRIORITY_MEDIUM, TINBUS_DATA_FRAME);

                uint8_t buffer[MESSAGE_SIZE];
                buffer[0] = (uint8_t)(msg_id >> 24);
                buffer[1] = (uint8_t)(msg_id >> 16);
                buffer[2] = (uint8_t)(msg_id >> 8);
                buffer[3] = (uint8_t)(msg_id);

                buffer[4] = DATA_SIZE; // data size (dlc)

                buffer[5] = (uint32_t)current >> 10;
                buffer[6] = (uint32_t)current >> 2;
                current = 0;

                buffer[7] = (uint64_t)charge >> 26;
                buffer[8] = (uint64_t)charge >> 18;
                buffer[9] = (uint64_t)charge >> 10;
                buffer[10] = (uint64_t)charge >> 2;

                uint8_t crc = 0;
                for (uint8_t i = 0; i < MESSAGE_SIZE - 1; i++) {
                    crc = crc8(crc, buffer[i]);
                }

                buffer[MESSAGE_SIZE - 1] = crc;

                uint8_t tries = 0;
                while (tries < 5) {
                    if (send(buffer, MESSAGE_SIZE)) {
                        break;
                    }
                    tries++;
                    uint8_t i = tries + (TCNT0 & 0x07); // add some randomness
                    while (i--) {
                        _delay_ms(1);
                    }
                }
            }
        }
    }

    return 0;
}
