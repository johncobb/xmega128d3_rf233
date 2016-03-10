/*
 * radio_sleeping.h
 *
 * Created: 7/12/2012 12:39:20 PM
 *  Author: Eric Rudisill
 */ 


#ifndef RADIO_SLEEPING_H_
#define RADIO_SLEEPING_H_


void radio_sleeping_enter(void);
void radio_sleeping_irq(uint8_t status);
void radio_sleeping_exit(void);



#endif /* RADIO_SLEEPING_H_ */