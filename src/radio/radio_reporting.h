/*
 * radio_reporting.h
 *
 * Created: 7/9/2012 3:11:06 PM
 *  Author: Eric Rudisill
 */ 


#ifndef RADIO_REPORTING_H_
#define RADIO_REPORTING_H_


void radio_reporting_enter(void);
void radio_reporting_tick(void);
void radio_reporting_irq(uint8_t status);


#endif /* RADIO_REPORTING_H_ */