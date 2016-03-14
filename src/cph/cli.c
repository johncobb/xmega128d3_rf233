/*
 * cli.c
 *
 *  Created on: Mar 11, 2016
 *      Author: jcobb
 */

#include "cph.h"
#include "cli.h"
#include "radio.h"


volatile char debug_in = '\0';


void debug_cb(uint8_t data)
{
	debug_in = data;
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
			radio_set_clock(CLKM_NO_CLOCK);
			printf_P(PSTR("RF233 CLKM turned off\r\n"));
		}
		else if (debug_in == 'C')
		{
			radio_set_clock(CLKM_8MHZ);
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
