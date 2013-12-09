#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <stdlib.h>
//#include <avr/iotn84a.h>

#define LOW_DUTY 1
#define HIGH_DUTY 220
#define DELAY_TICKS_MIN 20
#define DELAY_TICKS_MAX 100 - DELAY_TICKS_MIN
#define DELAY_TICKS_STARTUP_DISPLAY 50
#define BLINK_HZ 4
#define BLINK_TIME (F_CPU/(512 * BLINK_HZ)) - 1
#define BLINK_DURATION 10
#define FDIR 0
#define CHEK 1
#define DELY 2


volatile uint8_t blinkCounter;

__inline void setupTimer0()
{
    PRR &= ~_BV(PRTIM0);
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS02);
    OCR0A = 127;
    TIMSK0 = _BV(OCIE0A);
}

__inline void setupMisc() { PRR = _BV(PRUSI); }
__inline void configPins()
{
    DDRA |= _BV(DDA5) | _BV(DDA6);
    DIDR0 = 0b11111111;
    PORTB |= _BV(PORTB0) | _BV(PORTB1) | _BV(PORTB2);
}

__inline void setPinsLow() { PORTA &= ~_BV(PORTA5) | ~_BV(PORTA6); }

__inline void shutOffLEDPWM()
{
    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A = 0;
    OCR1B = 0;
    PRR |= _BV(PRTIM1);
}

__inline void shutOffTimer0()
{
    TIMSK0 = 0;
    TCCR0A = 0;
    TCCR0B = 0;
    OCR0A = 0;
    PRR |= _BV(PRTIM0);
}

__inline void setupADC()
{
    ACSR |= _BV(ACIE);
}

__inline void comparIRQ_Off()
{
    ACSR &= ~_BV(ACIE);
    ACSR |= _BV(ACI);
}

__inline void comparDisable() { ACSR |= _BV(ACD); }
__inline void comparEnable() { ACSR &= ~_BV(ACD); }
__inline void comparIRQ_On() { ACSR |= _BV(ACIE); }
__inline volatile uint8_t isItDay() { return !(ACSR & _BV(ACO)); }
__inline void set_ctcFlag() { GPIOR0 |= _BV(CHEK); }
__inline void clr_ctcFlag() { GPIOR0 &= ~_BV(CHEK); }
__inline uint8_t is_ctcFlag() { return (GPIOR0 & _BV(CHEK)); }
__inline void setFadeIn() { GPIOR0 |= _BV(FDIR); }
__inline void setFadeOut() { GPIOR0 &= ~_BV(FDIR); }
__inline uint8_t isFadeIn() { return (GPIOR0 & _BV(FDIR)); }
__inline void incDuty() { OCR1A++; OCR1B++; }
__inline void decDuty() { OCR1A--; OCR1B--; }
__inline void setDelay() { GPIOR0 |= _BV(DELY); GPIOR1 = 0; }
__inline void clrDelay() { GPIOR0 &= ~_BV(DELY); }
__inline uint8_t isDelay() { return (GPIOR0 & _BV(DELY)); }
__inline void setupLEDPWM()
{
    PRR &= ~_BV(PRTIM1);
    clrDelay();
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
    TCCR1B = _BV(WGM12) | _BV(CS12);
    OCR1A = OCR1B = 0;
}

__inline void setupLEDBlink()
{
    TIMSK0 = 0;
    OCR1A = OCR1B = 0;
    TCCR1A = _BV(COM1A0) | _BV(COM1B0);
    TCCR1B = _BV(WGM12) | _BV(CS12);
    TIMSK1 = _BV(OCIE1A);
    OCR1A = OCR1B = BLINK_TIME;
    blinkCounter = 0;
    while (blinkCounter < BLINK_DURATION) { asm ("nop"); wdt_reset(); }
    TIMSK1 = 0;
    TIMSK0 = _BV(OCIE0A);
}

void goToSleep();
void nothing();
#endif // DEFS_H_INCLUDED
