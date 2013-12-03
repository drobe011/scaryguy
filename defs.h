#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
//#include <avr/iotn84a.h>

#define LOW_DUTY 20
#define HIGH_DUTY 220
#define DELY_TICKS 50
#define FDIR 0
#define CHEK 1
#define DELY 2

__inline void setupTimer0()
{
    PRR &= ~_BV(PRTIM0);
    TCCR0A = _BV(WGM01);
    TCCR0B = _BV(CS02);
    OCR0A = 127;
    TIMSK0 = _BV(OCIE0A);
}

__inline void setupMisc() { PRR = _BV(PRUSI); }
__inline void configPins() { DDRA |= _BV(DDA5) | _BV(DDA6); }
__inline void setPinsLow() { PORTA &= ~_BV(PORTA5) | ~_BV(PORTA6); }

__inline void setupLEDPWM()
{
    PRR &= ~_BV(PRTIM1);
    TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM10);
    TCCR1B = _BV(WGM12) | _BV(CS10) | _BV(CS12);
    OCR1A = 0;
    OCR1B = 0;
}

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
    ACSR |= _BV(ACIE) | _BV(ACIS0) | _BV(ACIS1);
    DIDR0 = _BV(ADC1D) | _BV(ADC2D);
}

__inline void acIRQ_Off()
{
    ACSR &= ~_BV(ACIE);
    ACSR |= _BV(ACI);
}

__inline void acIRQ_On() { ACSR |= _BV(ACIE); }
__inline uint8_t isItDay() { return (ACSR & _BV(ACO)); }
__inline void set_ctcFlag() { GPIOR0 |= _BV(CHEK); }
__inline void clr_ctcFlag() { GPIOR0 &= ~_BV(CHEK); }
__inline uint8_t is_ctcFlag() { return (GPIOR0 & _BV(CHEK)); }
__inline void setFadeIn() { GPIOR0 |= _BV(FDIR); }
__inline void setFadeOut() { GPIOR0 &= ~_BV(FDIR); }
__inline uint8_t isFadeIn() { return (GPIOR0 & _BV(FDIR)); }
__inline void incDuty() { OCR1A++; OCR1B++; }
__inline void decDuty() { OCR1A--; OCR1B--; }
__inline void setDely() { GPIOR0 |= _BV(DELY); }
__inline void clrDely() { GPIOR0 &= ~_BV(DELY); }
__inline uint8_t isDely() { return (GPIOR0 & _BV(DELY)); }

void goToSleep();
void nothing();

void (*state)();

#endif // DEFS_H_INCLUDED
