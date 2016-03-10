
/*
 * debug.c
 *
 * Created: 6/25/2012 8:49:01 AM
 *  Author: Eric Rudisill
 */ 

#ifdef DEBUG

#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "debug.h"

const unsigned char DEBUG_hex[] PROGMEM = "0123456789ABCDEF";

debug_rx_cb_t debug_rx_cb;

// Forward
int _debug_putch(char data, FILE *dummy);

void _debug_init(debug_rx_cb_t cb) 
{ 
	// Set the TxD pin as an output - set DEBUG_PORT OUT register bit 3 to 1
	DEBUG_PORT.DIRSET = PIN3_bm; // as output
	DEBUG_PORT.DIRCLR = PIN2_bm;	// as input


	// Transmitter is enabled 
	// Set TxD=1 
	DEBUG_PORT.OUTSET=PIN3_bm; 

	// Communication mode: Asynchronous USART 
	// Data bits: 8 
	// Stop bits: 1 
	// Parity: Disabled 
	DEBUG_USART.CTRLC = USART_CMODE_ASYNCHRONOUS_gc |
					USART_PMODE_DISABLED_gc | 
					USART_CHSIZE_8BIT_gc; 

	// Receive complete interrupt: Disabled 
	// Transmit complete interrupt: Disabled 
	// Data register empty interrupt: Disabled 
	DEBUG_USART.CTRLA = (DEBUG_USART.CTRLA & (~(USART_RXCINTLVL_gm | USART_TXCINTLVL_gm | USART_DREINTLVL_gm))) | 
					USART_RXCINTLVL_HI_gc |
					USART_TXCINTLVL_OFF_gc | 
					USART_DREINTLVL_OFF_gc; 

	// 115200, 2MHz - calculated from http://prototalk.net/forums/showthread.php?t=188
	//int bsel = 5;
	//uint8_t bscale = 10;

	// 115200, 8MHz - calculated from http://prototalk.net/forums/showthread.php?t=188 
	//int bsel = 214;
	//uint8_t bscale = 10;
	
	// 115200, 16MHz - calculated from http://prototalk.net/forums/showthread.php?t=188
	int bsel = 492;
	uint8_t bscale = 10;

	DEBUG_USART.BAUDCTRLA = (uint8_t) bsel;
	DEBUG_USART.BAUDCTRLB = (bscale << 4) | (bsel >> 8);

	// Receiver: On 
	// Transmitter: On 
	// Double transmission speed mode: Off 
	// Multi-processor communication mode: Off 
	DEBUG_USART.CTRLB = (DEBUG_USART.CTRLB & (~(USART_RXEN_bm | USART_TXEN_bm | USART_CLK2X_bm | USART_MPCM_bm | USART_TXB8_bm))) | 
					USART_RXEN_bm |
					USART_TXEN_bm; 
					
	// Set the callback
	_debug_set_rx_cb(cb);
	
	//Redirect stdio so we can use printf/puts/etc.
	//TODO: Implement getch for stdio
    fdevopen(_debug_putch, NULL);
					
} 

ISR(DEBUG_ISR_VECTOR)
{
	char data = DEBUG_USART.DATA;
	if (debug_rx_cb != 0) debug_rx_cb(data);
}

void _debug_set_rx_cb(debug_rx_cb_t cb)
{
	debug_rx_cb = cb;
}

int _debug_putch(char data, FILE *dummy)
{	
	_debug_byte((uint8_t) data);	
	return 0;
}

void _debug_stringz( char* data )
{
	unsigned char c = *data;
	
	while (c) {
		while ( !( DEBUG_USART.STATUS & USART_DREIF_bm) );
		DEBUG_USART.DATA = c;
		c = *(++data);
	}
}

void _debug_byte(uint8_t b)
{
	while ( !( DEBUG_USART.STATUS & USART_DREIF_bm) );
	DEBUG_USART.DATA = b;
}


void _debug_byte_as_hex( unsigned char byte )
{
	unsigned char c;
	
	c = pgm_read_byte(&DEBUG_hex[(byte >> 4)]);
	while ( !( DEBUG_USART.STATUS & USART_DREIF_bm) );
	DEBUG_USART.DATA = c;
	
	c = pgm_read_byte(&DEBUG_hex[(byte & 0x0f)]);
	while ( !( DEBUG_USART.STATUS & USART_DREIF_bm) );
	DEBUG_USART.DATA = c;
}

#endif	/* DEBUG */