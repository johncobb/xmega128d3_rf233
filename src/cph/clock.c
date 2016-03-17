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
volatile clock_time_t wireless_sync_millis = 0;

bool EVSYS_SetEventSource( uint8_t eventChannel, EVSYS_CHMUX_t eventSource );

ISR(TCC0_CCA_vect) {

	if ( TCC0.INTFLAGS & TC0_CCAIF_bm ) {
		/*  Clear interrupt flag when new value is captured.
		 *  The last capture value is now available
		 *  in the CCA register.
		 */
		TCC0.INTFLAGS |= TC0_CCAIF_bm;
		TCC1.INTFLAGS |= TC1_CCAIF_bm;


		uint16_t word_high = TCC1.CCA;
		uint16_t word_low = TCC0.CCA;
		uint32_t timestamp = ((uint32_t) word_high << 16) | word_low;

	}
}

ISR(TCC0_OVF_vect){

	clock_millis++;
}

//ISR(TCD0_OVF_vect){
//	clock_millis++;
//}

//ISR(TCF0_OVF_vect){
//	clock_millis++;
//}

ISR(TCC1_OVF_vect) {
	wireless_sync_millis++;
}

void init_timer()
{
	TCC0.PER = 16000; // 2 MHz divided by 1000 to get milliseconds
	TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
	TCC0.INTCTRLA = ( TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;
}

void init_sync_timer()
{

//	TCC1.PER = 1; // 6.25 microsecond timer - sync_count
	TCC1.PER = 0; // 6.25 microsecond timer - sync_count
	TCC1.CTRLA = ( TCC1.CTRLA & ~TC1_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
	TCC1.INTCTRLA = ( TCC1.INTCTRLA & ~TC1_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;
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

	/* Setup a millisecond timer - clock_millis */
	TCC0.PER = 16000; // 16 MHz divided by 1000 to get milliseconds
	TCC0.CTRLA = ( TCC0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
	TCC0.INTCTRLA = ( TCC0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;

//	TCD0.PER = 16000; // 16 MHz divided by 1000 to get milliseconds
//	TCD0.CTRLA = ( TCD0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
//	TCD0.INTCTRLA = ( TCD0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;

//	TCF0.PER = 16000; // 16 MHz divided by 1000 to get milliseconds
//	TCF0.CTRLA = ( TCF0.CTRLA & ~TC0_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
//	TCF0.INTCTRLA = ( TCF0.INTCTRLA & ~TC0_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;



	
//	TCC1.PER = 1; // 6.25 microsecond timer - sync_count
//	TCC1.CTRLA = ( TCC1.CTRLA & ~TC1_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
//	TCC1.INTCTRLA = ( TCC1.INTCTRLA & ~TC1_OVFINTLVL_gm ) | TC_OVFINTLVL_LO_gc;


}


void clock_init_32bit_counter(void)
{
	/* Configure PD0 as input, sense on both edges. */
	PORTD.PIN0CTRL = PORT_ISC_BOTHEDGES_gc;
	PORTD.DIRCLR = 0x01;


	/* Select TCC0 overflow as event channel 0 multiplexer input.
	 * (Overflow propagation)
	 */
	EVSYS_SetEventSource( 0, EVSYS_CHMUX_TCC0_OVF_gc );

	/* Select PD0 as event channel 1 multiplexer input. (Input capture) */
	EVSYS_SetEventSource( 1, EVSYS_CHMUX_PORTD_PIN0_gc );

	/* Select event channel 0 as clock source for TCC1. */
	TCC1.CTRLA = TC_CLKSEL_EVCH0_gc;

	/* Configure TCC0 for input capture. */
	TCC0.CTRLD = (uint8_t) TC_EVSEL_CH1_gc | TC_EVACT_CAPT_gc;

	/* Configure TCC1 for input capture with event delay. */
	TCC1.CTRLD = (uint8_t) TC_EVSEL_CH1_gc | TC0_EVDLY_bm | TC_EVACT_CAPT_gc;

	/* Enable Compare or Capture Channel A for both timers. */
	TCC0.CTRLB = TC0_CCAEN_bm;
	TCC1.CTRLB = TC1_CCAEN_bm;

	/* Select system clock as clock source for TCC0. */
	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;

}

/*! \brief This function sets the event source for an event channel.
 *
 *  \param eventChannel     The event channel number, range 0-7.
 *  \param eventSource      The event source to use as input to the MUX.
 *
 *  \retval true  if a valid channel was selected.
 *  \retval false if a non-valid channel was selected.
 */
bool EVSYS_SetEventSource( uint8_t eventChannel, EVSYS_CHMUX_t eventSource )
{
	volatile uint8_t * chMux;

	/*  Check if channel is valid and set the pointer offset for the selected
	 *  channel and assign the eventSource value.
	 */
	if (eventChannel < 8) {
		chMux = &EVSYS.CH0MUX + eventChannel;
		*chMux = eventSource;

		return true;
	} else {
		return false;
	}
}


/*! \brief Example 3 from the application note.
 *
 *  This function implements example 3, "32-bit Timer/Counter with 32-bit
 *  Input Capture" from the "Examples" section of the application note AVR1001.
 *
 *  The example shows how to set up Timer/Counter TCC0 and TCC1 for 32 bit input
 *  capture, triggered by a logic change on PD0. The corresponding interrupt
 *  flag is set when a capture has occured and the capture time is put in
 *  the CCA register.
 */
//void Example3( void )
//{
//	/* Configure PD0 as input, sense on both edges. */
//	PORTD.PIN0CTRL = PORT_ISC_BOTHEDGES_gc;
//	PORTD.DIRCLR = 0x01;
//
//	/* Select TCC0 overflow as event channel 0 multiplexer input.
//	 * (Overflow propagation)
//	 */
//	EVSYS_SetEventSource( 0, EVSYS_CHMUX_TCC0_OVF_gc );
//
//	/* Select PD0 as event channel 1 multiplexer input. (Input capture) */
//	EVSYS_SetEventSource( 1, EVSYS_CHMUX_PORTD_PIN0_gc );
//
//	/* Select event channel 0 as clock source for TCC1. */
//	TCC1.CTRLA = TC_CLKSEL_EVCH0_gc;
//
//	/* Configure TCC0 for input capture. */
//	TCC0.CTRLD = (uint8_t) TC_EVSEL_CH1_gc | TC_EVACT_CAPT_gc;
//
//	/* Configure TCC1 for input capture with event delay. */
//	TCC1.CTRLD = (uint8_t) TC_EVSEL_CH1_gc | TC0_EVDLY_bm | TC_EVACT_CAPT_gc;
//
//	/* Enable Compare or Capture Channel A for both timers. */
//	TCC0.CTRLB = TC0_CCAEN_bm;
//	TCC1.CTRLB = TC1_CCAEN_bm;
//
//	/* Select system clock as clock source for TCC0. */
//	TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
//
//	while (1) {
//		if ( TCC0.INTFLAGS & TC0_CCAIF_bm ) {
//			/*  Clear interrupt flag when new value is captured.
//			 *  The last capture value is now available
//			 *  in the CCA register.
//			 */
//			TCC0.INTFLAGS |= TC0_CCAIF_bm;
//			TCC1.INTFLAGS |= TC1_CCAIF_bm;
//		}
//	}
//}



/*! \brief This example shows how to configure Timer/Counter for 32-bit counting
 *         with input capture.
 *
 *  This function implements example 6, "Setting up a 32-bit Timer/Counter With
 *  Input Capture" from the "Getting Started" section of application note
 *  AVR1306.
 *
 *  This example shows how to configure TCC0 and TCC1 for 32-bit Timer/Counter
 *  operation with input capture. The overflow from TCC0 is routed through
 *  event channel 0 to TCC1. An input capture is triggered by a falling edge on
 *  PC0, routed through event channel 1.
 */
//void Example6( void )
//{
//	uint32_t inputCaptureTime;
//
//	/* Configure PC0 for input, triggered on falling edge. */
//	PORTC.PIN0CTRL = PORT_ISC_FALLING_gc;
//	PORTC.DIRCLR = 0x01;
//
//	/* Configure PORTD as output. */
//	PORTD.DIRSET = 0xFF;
//
//	/* Use PC0 as multiplexer input for event channel 1. */
//	EVSYS.CH1MUX = EVSYS_CHMUX_PORTC_PIN0_gc;
//
//	/* Use TCC0 overflow as input for event channel 0. */
//	EVSYS.CH0MUX = EVSYS_CHMUX_TCC0_OVF_gc;
//
//	/*  Configure TCC0 and TCC1 for input capture with event channel 1 as
//	 *  trigger source.
//	 */
//	TC0_ConfigInputCapture( &TCC0, TC_EVSEL_CH1_gc );
//	TC1_ConfigInputCapture( &TCC1, TC_EVSEL_CH1_gc );
//
//	/* Enable event delay on TCC1. */
//	TC_EnableEventDelay( &TCC1 );
//
//	/* Enable input capture channel A on TCC0 and TCC1 */
//	TC0_EnableCCChannels( &TCC0, TC0_CCAEN_bm );
//	TC1_EnableCCChannels( &TCC1, TC1_CCAEN_bm );
//
//	/* Use event channel 0 as clock source for TCC1. */
//	TC1_ConfigClockSource( &TCC1, TC_CLKSEL_EVCH0_gc );
//
//	/* Select system clock as TCC0 clock source. */
//	TC0_ConfigClockSource( &TCC0, TC_CLKSEL_DIV1_gc );
//
//	do {
//		do {
//			/* Wait for Input Capture. */
//		} while ( TC_GetCCAFlag( &TCC0 ) == 0  );
//
//		uint16_t highWord = TC_GetCaptureA( &TCC1 );
//		uint16_t lowWord = TC_GetCaptureA( &TCC0 );
//		inputCaptureTime = ( (uint32_t) highWord << 16 ) | lowWord;
//
//		PORTD.OUT = (uint8_t) (inputCaptureTime >> 24);
//
//	} while (1);
//
//}
