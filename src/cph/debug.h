/*
 * debug.h
 *
 * Created: 12/6/2011 2:13:25 PM
 *  Author: Eric Rudisill
 */ 


#ifndef DEBUG_H_
#define DEBUG_H_

typedef void (*debug_rx_cb_t)(uint8_t);

#ifdef DEBUG

#define DEBUG_PORT       PORTE
#define DEBUG_USART      USARTE0
#define DEBUG_ISR_VECTOR USARTE0_RXC_vect

void _debug_init(debug_rx_cb_t);
void _debug_stringz( char* data );
void _debug_byte( unsigned char byte );
void _debug_byte_as_hex( unsigned char byte );
void _debug_set_rx_cb(debug_rx_cb_t);
#define debug_init(x) _debug_init(x);
#define debug_stringz(x) _debug_stringz(x);
#define debug_byte(x) _debug_byte(x);
#define debug_byte_as_hex(x) _debug_byte_as_hex(x);
#define debug_set_rx_cb(x) _debug_set_rx_cb(x);

extern unsigned char debug_buffer[128];

#else

#define debug_init(x) /**/
#define debug_stringz(x) /**/
#define debug_byte(x) /**/
#define debug_byte_as_hex(x) /**/
#define debug_set_rx_cb(x) /**/

#endif


#endif /* DEBUG_H_ */