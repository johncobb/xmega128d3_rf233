/*
 * radio_init.c
 *
 * Created: 7/6/2012 4:17:47 PM
 *  Author: Eric Rudisill
 */ 

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "rf233.h"
#include "radio.h"
#include "radio_state.h"

#define LOG(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
//#define LOG(fmt, ...) /*NO LOGGING*/

void radio_init_enter(void)
{
	memset(&RADIO_STATUS.RecentMessage, 0, sizeof(RADIO_STATUS.RecentMessage));

	uint8_t result = rf233_init();
	
	result = rf233_enable_tom();

	if (result == 0)
	{
		LOG("[INIT] FAILED\r\n");
	}
	else
	{
		LOG("[INIT] SUCCESS\r\n");
	}
	
	RADIO_STATUS.IsSleeping = 0;
	
	radio_set_state(LISTENING);
}
