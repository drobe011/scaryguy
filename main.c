/*
 */

#include "defs.h"

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));

void get_mcusr(void) \
  __attribute__((naked)) \
  __attribute__((section(".init3")));
void get_mcusr(void)
{
  mcusr_mirror = MCUSR;
  MCUSR = 0;
  wdt_disable();
}

int main(void)
{
    void (*state)();

    //SETUP PERIPHERALS, SLEEP AND WDT
    setupMisc();
    configPins();
    setupADC();
    setupTimer0();
    set_sleep_mode(SLEEP_MODE_IDLE);
    wdt_enable(WDTO_120MS);

    sei();

    setupLEDBlink();
    setupLEDPWM();
    //********************************

    while(1)
    {
        if (is_ctcFlag())
        {
            wdt_reset();
            comparEnable(); //TURN ON AC OPERATION
            if (GPIOR2 < DELAY_TICKS_MIN + 30)
            {
                setupLEDBlink();
                setupLEDPWM();
            }
            cli();

            state = (isItDay()) ? &goToSleep : &nothing;    //TEST FOR TWO STATES: NOTHINNG OR TIME TO SLEEP BECAUSE IT'S DAYTIME
            state();
            comparDisable();  //TURN OFF AC UNTIL NEXT CTC ON TIMER0
        }
    }
    return 0;
}

void goToSleep()
{
    //SHUTDOWN STUFF AND GO TO SLEEP (IDLE)
    wdt_disable();
    shutOffLEDPWM();
    shutOffTimer0();
    setPinsLow();
    comparIRQ_On();
    sleep_enable();
    sei();
    sleep_cpu();
    //WAKEUP AND START EVERYTHING BACK UP
    sleep_disable();
    comparIRQ_Off();
    setupLEDPWM();
    setupTimer0();
    wdt_enable(WDTO_120MS);
    clr_ctcFlag();
}

void nothing()
{
    clr_ctcFlag();
    sei();
}

ISR(ANA_COMP_vect)
{
    comparIRQ_Off();
    comparDisable();
}

ISR(TIM0_COMPA_vect)
{
    set_ctcFlag();

    if (isDelay())
    {
        if (GPIOR1 >= GPIOR2)
        {
            clrDelay();
            TCCR1A |= _BV(COM1A1) | _BV(COM1B1);
        }
        else GPIOR1++;
    }
    else
    {
        if (isFadeIn())
        {
            if (OCR1A < HIGH_DUTY) incDuty();
            else setFadeOut();
        }
        else
        {
            if (OCR1A >= LOW_DUTY) decDuty();
            else
            {
                GPIOR2 = (rand() / ((RAND_MAX / DELAY_TICKS_MAX + 1)) + DELAY_TICKS_MIN)*2; //GENERATE AND STORE RANDOM PAUSE BETWEEN FADE OUT/IN
                setFadeIn();
                GPIOR1 = 0;
                setDelay();
                OCR1A = OCR1B = 0;
                TCCR1A &= ~(_BV(COM1A1) | _BV(COM1B1));
           }
        }
    }
}

ISR(TIM1_COMPA_vect)
{
    blinkCounter++;
}
