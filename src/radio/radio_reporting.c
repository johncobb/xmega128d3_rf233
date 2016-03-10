/*
 * radio_reporting.c
 *
 * Created: 7/9/2012 3:10:24 PM
 *  Author: Eric Rudisill
 */ 

#include <avr/io.h>
#include "rf233.h"
#include "rf233_defs.h"
#include "radio.h"
#include "radio_state.h"

volatile uint8_t is_finished = 0;

void radio_reporting_enter(void)
{	
	is_finished = 0;
	rf233_set_trx_cmd(TRX_CMD_TX_ARET_ON);
}

void radio_reporting_tick(void)
{
	if (is_finished)
	{
		is_finished = 0;
		radio_set_state(LISTENING);
	}
}

void radio_reporting_irq(uint8_t status)
{
	if (status & IRQ_TRX_END_bm)
		is_finished = 1;
}