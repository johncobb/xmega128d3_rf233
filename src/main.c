/*
* TestRf233.c
*
* Created: 6/28/2012 3:37:45 PM
*  Author: Eric Rudisill
*/


#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "cph.h"
#include "clock.h"
#include "debug.h"
#include "radio.h"
#include "cli.h"

char autoping = 1;
char brief = 0;
char fast = 0;

void handle_clock_master(void);
void handle_clock_sync(void);

clock_time_t app_timer = 0;
clock_time_t app_period = 0;
static clock_time_t elapsed = 0;


void enable_interrupts(void)
{
	// Turn Interrupts on.
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	sei();
}


void receive_cb(radio_message_t * message)
{
//	printf_P(PSTR("receive_cb\r\n"));

}


/***********************************************************/
/*                                                         */
/*      M A I N                                            */
/*                                                         */
/***********************************************************/
int main(void)
{

//	app_period = DEFAULT_PERIOD_FAST;
	app_period = DEFAULT_PERIOD;
	
	// First thing: initialize clock
	clock_init();	
	
	PORTE.DIRSET = PIN0_bm;	//LED
	PORTE.OUTCLR = PIN0_bm;
	PORTE.DIRCLR = PIN4_bm;	//BUTTON
	
	debug_init(debug_cb);
	
	enable_interrupts();
	
	printf_P(PSTR("\r\nRF233 Test 2.0\r\n"));
	printf_P(PSTR("Press any key to start\r\n"));

	for(int i=0; i<3; i++) {
		PORTE.OUT = 0x01;
		_delay_ms(50);
		PORTE.OUT = 0x00;
		_delay_ms(50);
	}
	

	radio_init();



	radio_receive_cb = receive_cb;
	app_timer = clock_millis;


	// Force a command printout
	debug_in = '?';		
		
	while(1)
	{
		radio_tick();
		handle_input();

		if(CLOCK_MASTER == 1) {
			handle_clock_master();
		} else {
			handle_clock_sync();
		}


	}
}

void handle_clock_master(void)
{

	elapsed = clock_millis - app_timer;


	if (elapsed >= app_period)
	{
		radio_send_string("PING\r\n");
		app_timer = clock_millis;

	}
}

void handle_clock_sync(void)
{

}
