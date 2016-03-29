/*
 * clock.c
 *
 * Created: 6/27/2012 1:01:32 PM
 *  Author: Eric Rudisill
 */ 

#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "clock.h"

volatile clock_time_t clock_millis = 0;

ISR(TCC0_OVF_vect){

	clock_millis++;
}

void init_timer()
{
	TCC0.PER = 16000; // 2 MHz divided by 1000 to get milliseconds
	TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
	TCC0.INTCTRLA = ( TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;
}


/**
 * @brief Initialize the clock of the ATxmega
 */
void clock_init(void)
{
    /*
     * Enable 32MHz internal oscillator
     * (and by thus disable the 2 MHz internal oscillator). */
    OSC.CTRL = OSC_RC32MEN_bm;

    /*
     * The ATxmega shall run from its internal 32MHz Oscillator.
     * Set the clock speed to 16MHz. Use internal 32MHz and DFLL.
     */
    while (0 == (OSC.STATUS & OSC_RC32MRDY_bm))
    {
        /* Hang until the internal 32MHz Oscillator is stable. */
        ;
    }

#if (F_CPU == (32000000UL))
    /* Nothing to be done. */

#elif (F_CPU == (16000000UL))
    /* Enable change of protected IO register */
    CCP = 0xD8;
    /* Use Prescaler A to divide 32MHz clock by 2 to 16MHz system clock. */
    CLK.PSCTRL = CLK_PSADIV0_bm;

#elif (F_CPU == (8000000UL))
    /* Enable change of protected IO register */
    CCP = 0xD8;
    /* Use Prescaler A to divide 32MHz clock by 4 to 8MHz system clock. */
    CLK.PSCTRL = CLK_PSADIV0_bm | CLK_PSADIV1_bm;

#elif (F_CPU == (4000000UL))
    /* Enable change of protected IO register */
    CCP = 0xD8;
    /* Use Prescaler A to divide 32MHz clock by 8 to 4MHz system clock. */
    CLK.PSCTRL = CLK_PSADIV0_bm | CLK_PSADIV2_bm;

#else
//#   error Unknown system clock
#endif

    /* Enable change of protected IO register */
    CCP = 0xD8;
    /* Set internal 32MHz Oscillator as system clock. */
    CLK.CTRL = CLK_SCLKSEL0_bm;

    /* Enable DFLL for the internal oscillator. */
    OSC.DFLLCTRL = OSC_RC32MCREF_bm;
    DFLLRC32M.CTRL |= DFLL_ENABLE_bm;

	/* Setup a millisecond timer */
	TCC0.PER = 16000; // 2 MHz divided by 1000 to get milliseconds
	TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
	TCC0.INTCTRLA = ( TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;

}
