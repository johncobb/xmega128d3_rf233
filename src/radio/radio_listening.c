/*
 * radio_listening.c
 *
 * Created: 7/6/2012 4:44:04 PM
 *  Author: Eric Rudisill
 */ 

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "rf233.h"
#include "rf233_defs.h"
#include "radio.h"
#include "radio_state.h"
#include "radio_listening.h"

#define LOG(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
//#define LOG(fmt, ...) /*NO LOGGING*/

volatile uint8_t is_pending = 0;

void radio_listening_enter(void)
{
	is_pending = 0;
	rf233_set_trx_cmd(TRX_CMD_RX_ON);
}

void radio_listening_tick(void)
{
	if (is_pending)
	{
		radio_receive(&RADIO_STATUS.RecentMessage);
		is_pending = 0;
		if (radio_receive_cb != 0)
			radio_receive_cb(&RADIO_STATUS.RecentMessage);
	}
}

void radio_listening_irq(uint8_t status)
{
	if (status & IRQ_TRX_END_bm)
	is_pending = 1;
}
