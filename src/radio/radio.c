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
#include "cph.h"
#include "rf233.h"
#include "rf233_defs.h"
#include "radio.h"
#include "radio_state.h"
#include "radio_debug.h"
#include "radio_init.h"
#include "radio_listening.h"
#include "radio_reporting.h"
#include "radio_sleeping.h"

#define LOG(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
//#define LOG(fmt, ...) /*NO LOGGING*/

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

	//memset(&m, 0, sizeof(rf233_message_t));
	//m.PHR = RADIO_MSG_HDRLEN + length + RADIO_MSG_FCSLEN;			// (FCF & Seq#) + data + FCS
	int size = sizeof(rf233_message_t);
	m.PHR = size;
	
	m.PSDU[0] = 0x01;				// Data frame type, no security, frame pending, ack, pan compression
	m.PSDU[1] = 0x00;				// no src/dst addressing, IEEE 2003 compliant
	m.PSDU[2] = 0xAA; 				// Dummy sequence number
	int i = 0;
	for(i = 0; i < length; i++)
	{
		m.PSDU[3 + i] = *(data + i);
	}
	for(;i < RF233_PSDU_LENGTH - RADIO_MSG_HDRLEN; i++)
	{
		m.PSDU[3 + i] = 0x00;
	}
	
	LOG("[SND] PHR:%X DATA:%s\r\n", m.PHR, ((uint8_t *)m.PSDU) + RADIO_MSG_HDRLEN);
	//memcpy(((uint8_t*)m.PSDU) + RADIO_MSG_HDRLEN, data, 114);
	radio_set_state(REPORTING);

	rf233_send_message(&m);
	
	return 1;
}

//uint8_t radio_send_bytes(uint8_t *data, uint8_t length)
//{
//	rf233_message_t m;
//
//	memset(&m, 0, sizeof(rf233_message_t));
//	m.PHR = RADIO_MSG_HDRLEN + length + RADIO_MSG_FCSLEN;			// (FCF & Seq#) + data + FCS
//
//	m.PSDU[0] = 0x01;				// Data frame type, no security, frame pending, ack, pan compression
//	m.PSDU[1] = 0x00;				// no src/dst addressing, IEEE 2003 compliant
//	m.PSDU[2] = 0xAA; 				// Dummy sequence number
//
//	memcpy(((uint8_t*)m.PSDU) + RADIO_MSG_HDRLEN, data, length);
//
//	LOG("[SND] PHR:%X DATA:%s\r\n", m.PHR, ((uint8_t *)m.PSDU) + RADIO_MSG_HDRLEN);
//
//	radio_set_state(REPORTING);
//
//	rf233_send_message(&m);
//
//	return 1;
//}

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

	uint8_t frame_len;
	uint8_t *frame_ptr = NULL;

	// get the frame length
	rf233_frame_read(&frame_len, 1);

	rf233_message_t* rfm;
	memset(&rfm, 0, sizeof(rf233_message_t));

	if(frame_len > 127) {
		LOG("[FRAME_LEN_EXCEEDED] LEN:%02x\r\n", frame_len);
		return 1;
	}

	//Read data into a temporary buffer and cast it to rf233_message_t for easier data access
//	uint8_t temp_buff[sizeof(rf233_message_t)];
//	rf233_frame_read(temp_buff, sizeof(rf233_message_t));
//	rfm = (rf233_message_t*)temp_buff;

	//Read explicitly the data index for TIM 1,2,3 and bitshift them and display the data
	uint8_t temp_buff[sizeof(rf233_message_t)];
	rf233_frame_read(temp_buff, sizeof(rf233_message_t));

	//Print Raw Data
	LOG("Raw data:\r\n");
	for(int i = 0; i < sizeof(rf233_message_t); i++)
	{
		LOG("%d : %02x\r\n", i, temp_buff[i]);
	}
	LOG("\r\n");

//	rfm = (rf233_message_t*)temp_buff;
//	LOG("TOM 2, 1, 0: %02x %02x %02x\r\n", rfm->PSDU[0x7F], rfm->PSDU[0x7E], rfm->PSDU[0x7D]);
//	//Get numbers by bitshifting in values
//	uint32_t ToF = (((rfm->PSDU[0x7F]) << 16) + ((rfm->PSDU[0x7E]) << 8) + rfm->PSDU[0x7D]);
//	//ToF = (Bitshifted number/Max value of 24bit) * speed of light in air[m/s]
//	ToF = (ToF/pow(2,24)) * 299700000;
//	LOG("ToF: %lu meters\r\n", ToF);
//	//Frequency offset calculation = FEC x (500000/128)/fRF[MHZ]
//	int RFoff = (rfm->PSDU[0x7C])*(500000/128)/2400;
//	LOG("RF offset estimation: %d parts per million\r\n", RFoff);

	/*NOTE: These two functions read data directly into PSDU, meaning it won't make use of the struct formatting*/

	//Read frame for frame_len size into PSDU, and log the message byte by byte
//	rf233_frame_read(rfm.PSDU, frame_len);
//	LOG("[RCV] DATA: ");
//	for(int i = 0; i < frame_len; i++)
//	{
//		LOG("%02x", rfm.PSDU[i]);
//	}
//	LOG("\r\n");

	//Read the frame for all 128 bytes, so we can grab the TOM data off the end
//	uint32_t ToF = 0;
//	rf233_frame_read(rfm->PSDU, 127);
//	LOG("[RCV] DATA: ");
//	for(int i = 0x73; i < 0x7C; i++)
//	{
//		LOG("%02x", rfm->PSDU[i]);
//	}
//	LOG("%d", ToF);
//	LOG("\r\n");

	/*END NOTE*/


	return 0;

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
	LOG("[RCV] TOM_0: %02x TOM_1: %02x TOM_2: %02x\r\n");

	
	LOG("[RCV] PHR:%X FCS:%X LQI:%X ED:%X RX_STATUS:%X DATA:%s\r\n", m.PHR, m.FCS, m.LQI, m.ED, m.RX_STATUS, ((char*)m.PSDU) + RADIO_MSG_HDRLEN);
	
	return m.PHR;	
}

