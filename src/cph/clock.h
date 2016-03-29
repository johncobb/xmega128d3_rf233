/*
 * clock.h
 *
 * Created: 6/27/2012 1:02:41 PM
 *  Author: Eric Rudisill
 */ 


#ifndef CLOCK_H_
#define CLOCK_H_

#include <avr/io.h>

typedef uint32_t clock_time_t;
void init_timer();
void clock_init(void);
void clock_init_32bit_counter(void);
void init_sync_timer(void);

extern volatile clock_time_t clock_millis;



#endif /* CLOCK_H_ */
