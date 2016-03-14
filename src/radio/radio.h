/*
 * radio.h
 *
 * Created: 7/2/2012 12:35:43 PM
 *  Author: Eric Rudisill
 */ 


#ifndef RADIO_H_
#define RADIO_H_

#include "clock.h"
#include "rf233.h"
#include "rf233_defs.h"

#define RADIO_MSG_HDRLEN	(3)
#define RADIO_MSG_FCSLEN	(2)
#define RADIO_MSG_MAXLEN	(127 - RADIO_MSG_HDRLEN - RADIO_MSG_FCSLEN)

extern volatile uint32_t wireless_sync_counter;

typedef struct {
	uint16_t MacHeader;
	uint8_t Sequence;
	uint8_t Length;
	uint8_t Data[RADIO_MSG_MAXLEN];
	uint16_t Checksum;
	int8_t SignalStrength;
	uint8_t SignalStrengthRaw;		
} radio_message_t;

typedef struct {	
	uint8_t IsSleeping;
	radio_message_t RecentMessage;	
} radio_status_t;

typedef struct {
	uint8_t msg_type;
	clock_time_t clock;
	uint8_t temp;
} clock_sync_t;

typedef void (*radio_receive_cb_t)(radio_message_t *);

void tc_reset(void);

void radio_init(void);
void radio_set_clock(uint8_t new_clock);
void radio_tick(void);
void radio_print_status(void);
void radio_sleep(void);
void radio_wake(void);
uint8_t radio_send_clocksync(clock_sync_t *clock);
uint8_t radio_send_bytes(uint8_t *data, uint8_t length);
uint8_t radio_send_string(char *data);
uint8_t radio_receive(radio_message_t *);

extern radio_status_t RADIO_STATUS;
extern radio_receive_cb_t radio_receive_cb;

#endif /* RADIO_H_ */
