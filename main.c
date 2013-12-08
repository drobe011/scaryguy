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
    setupLEDPWM();
    setupADC();
    setupTimer0();

    set_sleep_mode(SLEEP_MODE_IDLE);
    wdt_enable(WDTO_120MS);
    //********************************

    sei();

    stateChangeDisplay(DELAY_TICKS_STARTUP_DISPLAY);

    while(1)
    {
        if (is_ctcFlag())
        {
            wdt_reset();
            comparEnable(); //TURN ON AC OPERATION
            setMode((GPIOR2 < DELAY_TICKS_MIN + 5) ? BLINK_MODE : PWM_MODE);
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

void delayNflash(uint8_t o1a, uint8_t o1b, uint8_t dly)
{
    OCR1A = o1a;
    OCR1B = o1b;
    GPIOR2 = dly;
    setDelay();
    while (GPIOR1 < GPIOR2) wdt_reset();
    OCR1A = 0;
    OCR1B = 0;
}

void stateChangeDisplay(uint8_t delayTime)
{
    delayNflash(127, 0, delayTime);
    delayNflash(0, 0, delayTime);
    delayNflash(0, 127, delayTime);
    delayNflash(0, 0, delayTime);
}

ISR(ANA_COMP_vect)
{
    comparIRQ_Off();
}

ISR(TIM0_COMPA_vect)
{
    set_ctcFlag();
    if (isDelay())
    {
        if (GPIOR1 >= GPIOR2) clrDelay();
        else GPIOR1++;
    }
    else if (isPWM())
    {
        if (isFadeIn())
        {
            if (OCR1A < HIGH_DUTY) incDuty();
            else setFadeOut();
        }
        else
        {
            if (OCR1A > LOW_DUTY) decDuty();
            else
            {
                GPIOR2 = (rand() / ((RAND_MAX / DELAY_TICKS_MAX + 1)) + DELAY_TICKS_MIN); //GENERATE AND STORE RANDOM PAUSE BETWEEN FADE OUT/IN
                setFadeIn();
                setDelay();
                OCR1A = OCR1B = 0;
            }
        }
    }
}
