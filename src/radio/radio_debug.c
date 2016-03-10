/*
 * radio_debug.c
 *
 * Created: 7/9/2012 3:50:35 PM
 *  Author: Eric Rudisill
 */ 

#include <stdio.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

//#define LOG(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
#define LOG(fmt, ...) /*NO LOGGING*/


// Collector for statuses
volatile uint8_t irq_status[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
volatile uint8_t irq_index = 0;	
volatile uint8_t irq_ovf = 0;

void reset_status_list(void)
{
	irq_index = 0;
	for (int i=0;i<sizeof(irq_status);i++)
	irq_status[i] = 0;
	irq_ovf = 0;
}

void radio_debug_tick(void)
{
	if (irq_index > 0)
	{
		LOG("[IRQ] ");
		for (int i=0;i<sizeof(irq_status);i++)
			LOG("%X ", irq_status[i]);
		if (irq_ovf)
			LOG(" OVF");
		LOG("\r\n");
		reset_status_list();
	}
}

void radio_debug_irq(uint8_t status)
{
	// Collect the status for debugging
	if (irq_index >= sizeof(irq_status))
	{
		irq_ovf = 0xFF;
		return;
	}		
	irq_status[irq_index++] = status;
}