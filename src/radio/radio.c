/*
* radio.c
*
* Created: 7/2/2012 12:33:50 PM
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
#include "radio_debug.h"
#include "radio_init.h"
#include "radio_listening.h"
#include "radio_reporting.h"
#include "radio_sleeping.h"

//#define LOG(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
#define LOG(fmt, ...) /*NO LOGGING*/

// Public fields
radio_status_t RADIO_STATUS;
radio_receive_cb_t radio_receive_cb = 0;

// State management
state_table_t STATE_TABLES [] = {
	{
		INITIALIZING, 
		radio_init_enter, 
		0, 
		0, 
		0
	},
	{
		LISTENING, 
		radio_listening_enter, 
		radio_listening_tick, 
		radio_listening_irq, 
		0
	},
	{
		SLEEPING, 
		radio_sleeping_enter, 
		0, 
		radio_sleeping_irq, 
		radio_sleeping_exit
	},
	{
		REPORTING, 
		radio_reporting_enter, 
		radio_reporting_tick, 
		radio_reporting_irq, 
		0
	},
	{
		REPEATING, 
		0, 
		0, 
		0, 
		0
	}
};
state_table_t * CURRENT_STATE = 0;

state_table_t DEBUG_STATE = {
	DEBUGGING,
	0,
	radio_debug_tick,
	radio_debug_irq,
	0
};



// *************************************
// ******* MAIN STATE FUNCTIONS ********
// *************************************

void handle_rf_irq(uint8_t status)
{


	// Report the IRQ
	if (CURRENT_STATE->interrupt)
		CURRENT_STATE->interrupt(status);

	// Debug the IRQ
	DEBUG_STATE.interrupt(status);		
}

void radio_init(void)
{
	rf233_irq_cb = handle_rf_irq;	
	radio_set_state(INITIALIZING);
}

void radio_set_clock(uint8_t new_clock)
{
	rf233_set_clock(new_clock);
}

void radio_tick(void)
{
	DEBUG_STATE.tick();
	
	if (CURRENT_STATE->tick)
		CURRENT_STATE->tick();
}

void radio_set_state(STATES new_state)
{
	if (CURRENT_STATE)
	{
		// Bail if we're already in this state
		if (CURRENT_STATE->current_state == new_state)
			return;
			
		if (CURRENT_STATE->exit)
		{
			LOG("[TRN] EXITING %X\r\n", CURRENT_STATE->current_state);
			CURRENT_STATE->exit();
		}
	}
	
	CURRENT_STATE = &STATE_TABLES[new_state];
	
	if (CURRENT_STATE->enter)
	{
		LOG("[TRN] ENTERING %X\r\n", CURRENT_STATE->current_state);
		CURRENT_STATE->enter();
	}
}



/*
 * Public Functions .. Accessible by state handlers as well.
 *                     (Yeah, I know - circular references! Bleh!)
 *
 */

void radio_print_status(void)
{
	rf233_status();
}

void radio_sleep(void)
{
	radio_set_state(SLEEPING);
}

void radio_wake(void)
{
	radio_set_state(LISTENING);
}

uint8_t radio_send_bytes(uint8_t *data, uint8_t length)
{
	rf233_message_t m;

	memset(&m, 0, sizeof(rf233_message_t));
	m.PHR = RADIO_MSG_HDRLEN + length + RADIO_MSG_FCSLEN;			// (FCF & Seq#) + data + FCS
	
	m.PSDU[0] = 0x01;				// Data frame type, no security, frame pending, ack, pan compression
	m.PSDU[1] = 0x00;				// no src/dst addressing, IEEE 2003 compliant
	m.PSDU[2] = 0xAA;				// Dummy sequence number
	
	memcpy(((uint8_t*)m.PSDU) + RADIO_MSG_HDRLEN, data, length);

	LOG("[SND] PHR:%X DATA:%s\r\n", m.PHR, ((uint8_t *)m.PSDU) + RADIO_MSG_HDRLEN);

	radio_set_state(REPORTING);

	rf233_send_message(&m);
	
	return 1;
}

uint8_t radio_send_string(char *data)
{
	uint8_t len = strlen(data);
	return radio_send_bytes((uint8_t*)data, len);
}

uint8_t radio_send_clocksync(clock_sync_t *clock)
{
	return radio_send_bytes(clock, sizeof(clock_sync_t));
}

uint8_t radio_receive(radio_message_t *rm)
{
	rf233_message_t m;
	
	memset(&m, 0, sizeof(rf233_message_t));
	memset(rm, 0, sizeof(radio_message_t));
	
	rf233_get_message(&m);
	
	rm->MacHeader = ((uint16_t)m.PSDU[0]) << 8;
	rm->MacHeader |= m.PSDU[1];
	rm->Sequence = m.PSDU[2];
	rm->Length = m.PHR;
	memcpy(rm->Data, ((char*)m.PSDU) + RADIO_MSG_HDRLEN, m.PHR);
	rm->Checksum = m.FCS;
	rm->SignalStrengthRaw = m.ED;
	rm->SignalStrength = RF233_RSSI_BASE + m.ED;
	
	LOG("[RCV] PHR:%X FCS:%X LQI:%X ED:%X RX_STATUS:%X DATA:%s\r\n", m.PHR, m.FCS, m.LQI, m.ED, m.RX_STATUS, ((char*)m.PSDU) + RADIO_MSG_HDRLEN);
	
	return m.PHR;	
}

