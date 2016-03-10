/*
 * radio_sleeping.c
 *
 * Created: 7/12/2012 12:38:47 PM
 *  Author: Eric Rudisill
 */ 

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "rf233.h"
#include "rf233_defs.h"
#include "radio.h"
#include "radio_state.h"
#include "radio_sleeping.h"

#define LOG(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
//#define LOG(fmt, ...) /*NO LOGGING*/

#define WAKING_STATE_NONE		(0)
#define WAKING_STATE_WAKING		(1)
#define WAKING_STATE_FINISHED	(2)

volatile uint8_t waking_state = WAKING_STATE_NONE;

void radio_sleeping_enter(void)
{
	waking_state = 0;
	rf233_set_trx_cmd(TRX_CMD_TRX_OFF);	
	SET_AUX(RF233_AUX_SLP_TR);
	RADIO_STATUS.IsSleeping = 0xFF;
}

void radio_sleeping_irq(uint8_t status)
{
	if (waking_state == WAKING_STATE_WAKING)
		if (status & IRQ_AWAKE_END_bm)
			waking_state = WAKING_STATE_FINISHED;
}

void radio_sleeping_exit(void)
{
	waking_state = WAKING_STATE_WAKING;
	RESET_AUX(RF233_AUX_SLP_TR);

	// wait for the radio to wake up
	while (waking_state == WAKING_STATE_WAKING) ;

	RADIO_STATUS.IsSleeping = 0;	
}
