/*
 * radio_listening.h
 *
 * Created: 7/6/2012 4:44:14 PM
 *  Author: Eric Rudisill
 */ 


#ifndef RADIO_LISTENING_H_
#define RADIO_LISTENING_H_


void radio_listening_enter(void);
void radio_listening_tick(void);
void radio_listening_irq(uint8_t status);


#endif /* RADIO_LISTENING_H_ */