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

#define SHUNT_DEVICE_ID 0x04

#define ADC_SAMPLES 7
// #define ADC_SAMPLES 1
#define BIT_PERIOD_US (104 - 12) // 9600 bps with clock at 921.6 kHz (4 us adjusted for loop delay)
#define BITS_PER_MESSAGE (10 * FRAME_SIZE * 4)

uint8_t getSwitch(void) {
    return (0x0F & (((uint8_t)(~PIND)) >> 3)); // read address from rotary switch
}

// returns true if successful
bool tx_zeros(uint8_t bit_count) {
    bool rx_inactive = false;
    if (RX_IS_RELEASED) {
        TX_ACTIVE;
        while (bit_count--) {
            _delay_us(BIT_PERIOD_US);
            if (!RX_IS_RELEASED) {
                rx_inactive = true;
            }
        }
        TX_RELEASE;
    }
    return rx_inactive;
}

// returns true if successful
bool tx_ones(uint8_t bit_count) {
    bool rx_inactive = true;
    while (bit_count--) {
        _delay_us(BIT_PERIOD_US);
        if (!RX_IS_RELEASED) {
            rx_inactive = false;
        }
    }
    return rx_inactive;
}

bool send(uint8_t data[], uint8_t bytes_to_send) {
    if (!tx_ones(20)) { // minimum 20 bit periods between bus activity
        return false;
    }
    while (bytes_to_send--) {
        uint8_t byte = *data++;
        uint8_t nibble = 4;
        while (nibble) {
            uint8_t zeros = 4 - (byte >> 6);
            if (!tx_zeros(zeros)) {
                return false;
            }
            if (!tx_ones(5 - zeros)) {
                return false;
            }
            byte <<= 2;
            nibble -= 1;
        }
    }
    return true;
}

int main(void) {
    static int32_t adc_average = 0;
    static uint8_t adc_count = 0;
    static bool frame_sent = false;
    static uint8_t frame_buffer[FRAME_SIZE] ={0};
    
    wdt_enable(WDTO_250MS);
    asm_init();

    DDRD |= (1 << PORTD1); // init bus outputs
    DDRD |= (1 << PORTD2);

    // DDRB |= (1 << PORTB2); // debug

    while (1) {
        wdt_reset();

        if (asm_ADC_doConversion()) // we now have about 150 ms before next adc interrupt
        {
            int32_t adc_value = asm_ADC_getRawResult();
            adc_average += adc_value;
            adc_count++;

            if (adc_count >= ADC_SAMPLES) {
                adc_count = 0;
                frame_buffer[0] = (SHUNT_DEVICE_ID << 4) | getSwitch();
                frame_buffer[1] = (uint8_t)((uint32_t)adc_average >> 16);
                frame_buffer[2] = (uint8_t)((uint32_t)adc_average >> 8);
                frame_buffer[3] = (uint8_t)((uint32_t)adc_average);
                adc_average = 0;
                uint8_t crc = 0;
                for (uint8_t index = 0; index < MESSAGE_SIZE; index++) {
                    crc = crc8(crc, frame_buffer[index]);
                }
                frame_buffer[MESSAGE_SIZE] = crc;
                frame_sent = false;
            }

            if (!frame_sent) {
                uint8_t send_attempts = 0;
                while (send_attempts < 5) {
                    if (send(frame_buffer, FRAME_SIZE)) {
                        frame_sent = true;
                        break;
                    }
                    send_attempts++;
                    uint8_t delay_bits = 0x80 + (TCNT0 & 0x3F);
                    while (delay_bits--) {
                        _delay_us(BIT_PERIOD_US);
                    }
                }
            }
        }
    }
    return 0;
}
