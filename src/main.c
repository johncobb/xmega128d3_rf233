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
#include "clock.h"
#include "debug.h"
#include "radio.h"
#include "rf233.h"
#include "rf233_defs.h"


#define DEFAULT_PERIOD	     (2000)
#define DEFAULT_PERIOD_FAST  (100)
#define WAIT_FOR_PRESS	{debug_in = '\0'; while (debug_in == '\0');}

volatile char debug_in = '\0';
clock_time_t timer = 0;
clock_time_t period = 0;

char autoping = 0;
char brief = 0;
char fast = 0;

void enable_interrupts(void)
{
	// Turn Interrupts on.
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	sei();
}

void debug_cb(uint8_t data)
{
	debug_in = data;
}

void receive_cb(radio_message_t * message)
{
	if (brief) {
		printf_P(PSTR("%X "), message->SignalStrengthRaw);
	}
	else
	{
		printf_P(PSTR("[RCV] MAC:%X SEQ:%X LEN:%X CS:%X SS:%d(%X) DATA: %s\r\n"), 
			message->MacHeader,
			message->Sequence,
			message->Length,
			message->Checksum,
			message->SignalStrength,
			message->SignalStrengthRaw,
			(char*)(message->Data));
	}		
}

void handle_input(void)
{
	if (debug_in != '\0')
	{
		if (debug_in == '?')
		{
			printf_P(PSTR("\r\n"));
			printf_P(PSTR("COMMANDS:    (+ == SHIFT)\r\n\r\n"));			
			printf_P(PSTR("  ENTER  - send HELLO WORLD\r\n"));
			printf_P(PSTR("  SPACE  - toggle sleep\r\n"));
			printf_P(PSTR("  a      - toggle 2 sec autoping PING\r\n"));
			printf_P(PSTR("  b      - toggle brief mode\r\n"));
			printf_P(PSTR("  c      - turn CLKM OFF\r\n"));
			printf_P(PSTR("  +c     - turn CLKM ON - 8 MHz\r\n"));
			printf_P(PSTR("  f      - toggle fast autoping\r\n"));
			printf_P(PSTR("  s      - print various RF233 status registers\r\n"));
			printf_P(PSTR("\r\n"));
		}
		else if (debug_in == 0x0d)
		{
			radio_send_string("HELLO WORLD");
		}
		else if (debug_in == ' ')
		{
			if (RADIO_STATUS.IsSleeping)
			{
				radio_wake();
				printf_P(PSTR("Radio is LISTENING\r\n"));
			}				
			else
			{
				radio_sleep();
				printf_P(PSTR("Radio is SLEEPING\r\n"));
			}				
		}
		else if (debug_in == 'a')
		{
			autoping = ~autoping;
			timer = clock_millis;
			printf_P(PSTR("AUTOPING is now %X\r\n"), autoping);
		}
		else if (debug_in == 'b')
		{
			brief = ~brief;
			printf_P(PSTR("BRIEF is now %X\r\n"), brief);
		}
		else if (debug_in == 'c')
		{
			rf233_set_clock(CLKM_NO_CLOCK);
			printf_P(PSTR("RF233 CLKM turned off\r\n"));
		}
		else if (debug_in == 'C')
		{
			rf233_set_clock(CLKM_8MHZ);
			printf_P(PSTR("RF233 CLKM turned ON (8 MHz)\r\n"));
		}
		else if (debug_in == 'f')
		{
			fast = ~fast;
			period = (fast ? DEFAULT_PERIOD_FAST : DEFAULT_PERIOD);
			printf_P(PSTR("FAST mode is %X\r\n"), fast);
		}
		else if (debug_in == 's')
		{
			radio_print_status();
		}
		
		debug_in = '\0';
	}
}


/***********************************************************/
/*                                                         */
/*      M A I N                                            */
/*                                                         */
/***********************************************************/
int main(void)
{
	clock_time_t elapsed = 0;
	period = DEFAULT_PERIOD;
	
	// First thing: initialize clock
	clock_init();	
	
	PORTE.DIRSET = PIN0_bm;	//LED
	PORTE.OUTCLR = PIN0_bm;
	PORTE.DIRCLR = PIN4_bm;	//BUTTON
	
	debug_init(debug_cb);
	
	enable_interrupts();
	
	printf_P(PSTR("\r\nRF233 Test 1.0\r\n"));
	printf_P(PSTR("Press any key to start\r\n"));


	for(int i=0; i<5; i++) {
		PORTE.OUT = 0x01;
		_delay_ms(50);
		PORTE.OUT = 0x00;
		_delay_ms(50);
	}
//	PORTE.OUT = 0x01;
//	_delay_ms(1000);
//	PORTE.OUT = 0x00;
	
#if AUTOPING
	autoping = 0xFF;
#else
	WAIT_FOR_PRESS
#endif
	
	radio_init();
	
	radio_receive_cb = receive_cb;
	timer = clock_millis;

	// Force a command printout
	debug_in = '?';		
		
	while(1)
	{
		radio_tick();

		handle_input();
		
		if (autoping)
		{
			elapsed = clock_millis - timer;
			if (elapsed >= period)
			{
				radio_send_string("PING");


				timer = clock_millis;
			}
		}		
	}
}
