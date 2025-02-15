#include <avr/io.h>
#include <avr/interrupt.h>

#include "timer.h"

volatile uint32_t timer_overflows = 0;

void timer_init(void)
{
    TCCR0B = (1 << CS02); // divide F_CPU by 256
    TIMSK0 = (1 << TOIE0);
}

uint32_t timer_get_ticks(void)
{
    cli();
    uint32_t overflows = timer_overflows;
    uint8_t tcnt0 = TCNT0;
    if ((TIFR0 & (1 << TOV0)) && (tcnt0 < 255)){
        overflows++;
    }
    sei();
    return (overflows << 8) + (uint32_t)tcnt0;
}

ISR(TIMER0_OVF_vect)
{
    timer_overflows++;
}
