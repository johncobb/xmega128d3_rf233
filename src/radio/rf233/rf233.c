/*
 * rf233.c
 *
 * Created: 6/26/2012 11:19:45 AM
 *  Author: Eric Rudisill
 */ 

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "clock.h"
#include "rf233.h"
#include "rf233_defs.h"

#define LOG_V(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
//#define LOG_V(fmt, ...) /*NO LOGGING*/

#define LOG(fmt, ...) printf_P(PSTR(fmt), ##__VA_ARGS__)
//#define LOG(fmt, ...) /*NO LOGGING*/

#define _send(x)		rf233_spi_putc(x)
#define _read(x)		rf233_spi_read_register(x)
#define _write(x,y)		rf233_spi_write_register(x,y)

rf233_irq_cb_t rf233_irq_cb;

clock_time_t wireless_sync_elapsed = 0;
clock_time_t wireless_sync_prev = 0;
uint8_t sync_received = 0;

// Forward decs
uint8_t rf233_spi_putc( uint8_t data );
uint8_t rf233_spi_read_register( uint8_t address);
uint8_t rf233_spi_write_register( uint8_t address, uint8_t data);



ISR(RF233_IRQ_VECTOR)
{
	//(6.25*12800)/1000
	wireless_sync_elapsed = wireless_sync_millis - wireless_sync_prev;
	wireless_sync_prev = wireless_sync_millis;
	sync_received = 1;


	uint8_t status = _read(IRQ_STATUS);
	if (rf233_irq_cb != 0) rf233_irq_cb(status);
}

uint8_t rf233_spi_putc( uint8_t data )
{
	// put byte in send-buffer
	RF233_SPI.DATA = data;
	
	// wait until byte was send
	while( !( RF233_SPI.STATUS & SPI_IF_bm ))	;
	
	uint8_t d = RF233_SPI.DATA;
	
	//LOG_V("RF233-PUT: %X %X\r\n", data, d);

	return d;
}

uint8_t rf233_spi_read_register( uint8_t address)
{
	LOG_V("RF233-RED: %X\r\n", address);
	
	uint8_t ret = 0;
	
	RESET_CS
	
	ret = rf233_spi_putc(READ_REGISTER | address);
	ret = rf233_spi_putc(0xFF);
	
	SET_CS
	
	return ret;
}

uint8_t rf233_spi_write_register( uint8_t address, uint8_t data)
{
	LOG_V("RF233-WRT: %X  %X\r\n", address, data);
	
	uint8_t ret = 0;
	
	RESET_CS
	
	ret = rf233_spi_putc(WRITE_REGISTER | address);
	ret = rf233_spi_putc(data);
	
	SET_CS
	
	return ret;
}

uint8_t rf233_init_io()
{
	LOG_V("rf233_init_io: ENTER\r\n");
	
	// Set pin directions on SPI port
	SET_OUTPUT(RF233_SPI_CS);
	SET_OUTPUT(RF233_SPI_MOSI);
	SET_INPUT(RF233_SPI_MISO);
	SET_OUTPUT(RF233_SPI_SCK);
	
	// Set pin directions on AUX port
	SET_OUTPUT_AUX(RF233_AUX_RST);
	SET_OUTPUT_AUX(RF233_AUX_SLP_TR);
	SET_INPUT_AUX(RF233_AUX_IRQ);
	SET_INPUT_AUX(RF233_AUX_DIG2);

	// Set the IRQ line as a true interrupt
	RF233_AUX_PORT.INTCTRL = PORT_INT0LVL_HI_gc;
	RF233_AUX_PORT.INT0MASK = (1 << RF233_AUX_IRQ);

	// Configure lines on SPI
	SET(RF233_SPI_CS)
	RESET(RF233_SPI_MOSI);
	RESET(RF233_SPI_MISO);
	RESET(RF233_SPI_SCK);
	
	// Configure lines on AUX and enable/reset the chip
	RESET_AUX(RF233_AUX_DIG2);
	RESET_AUX(RF233_AUX_IRQ);
	RESET_AUX(RF233_AUX_SLP_TR);
	SET_AUX(RF233_AUX_RST);
	
	// active SPI master interface  clk/4
	RF233_SPI.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm; 
	RF233_SPI.STATUS = 0;

	_delay_ms(10);

	LOG_V("rf233_init_io: EXIT\r\n");

	return RF233_TRUE;
}

uint8_t rf233_init(void)
{
	LOG_V("rf233_init: ENTER\r\n"); 
	
	// Setup all the MCU pins and SPI
	rf233_init_io();
	
	// Set clock to 8 MHz immediately, then let it settle
	rf233_set_clock(CLKM_8MHZ);
	
	// Enable various interrupts
	_read(IRQ_STATUS);	// recommended to clear history
	//_write(IRQ_MASK, (IRQ_RX_START_bm | IRQ_AMI_bm | IRQ_TRX_END_bm));
	_write(IRQ_MASK, (0xFF)); // turn them all on
	


	// Go to TRX_OFF state
	rf233_set_trx_cmd(TRX_CMD_TRX_OFF);
	
	LOG_V("rf233_init: EXIT\r\n");
	
	return RF233_TRUE;
}

uint8_t rf233_enable_tom(void)
{
	LOG_V("rf233_enable_tom: ENTER\r\n");

	// enable TOM: Time of Flight
	_write(0X03, 0b10101101);

	LOG_V("rf233_enable_tom: EXIT\r\n");

	return RF233_TRUE;
}

void rf233_status(void)
{
	uint8_t data = 0;
	
	LOG("[STA] ");
	
	data = RF233_AUX_PORT.IN;
	LOG("AUX_PORT.IN:%X ", data);
	
	data = _read(TRX_STATUS);
	LOG("TRX_STATUS:%X ", data);

	data = _read(TRX_STATE);
	LOG("TRX_STATE:%X ", data);

	data = _read(TRX_CTRL_0);
	LOG("TRX_CTRL_0:%X ", data);

	data = _read(IRQ_STATUS);
	LOG("IRQ_STATUS:%X ", data);
	
	data = _read(BATMON);
	LOG("BATMON:%X ", data);
	
	LOG("\r\n");
}

void rf233_reset(void)
{
	LOG_V("rf233_reset: ENTER\r\n");
	RESET_AUX(RF233_AUX_RST);
	_delay_ms(1);
	SET_AUX(RF233_AUX_RST);
	_delay_ms(10);
	LOG_V("rf233_reset: EXIT\r\n");
}

void rf233_set_clock(uint8_t new_clock)
{
	// Set clock immediately, then let it settle
	uint8_t r = CLKM_SHA_DISABLE | new_clock;
	_write(TRX_CTRL_0, r);
	_delay_ms(10);
}

void rf233_set_trx_cmd(uint8_t cmd)
{
	LOG_V("rf233_set_trx_cmd: ENTER  %X\r\n", cmd);
	
	// Send the command 
	_write(TRX_STATE, cmd);		// TRX_STATE => TRX_OFF
	
	// Wait for transition to occur
	uint8_t status = 0;
	while ( ((status = _read(TRX_STATUS)) & TRX_STATUS_bm) == STATE_TRANSITION_IN_PROGRESS) 
		_delay_us(1);	// give SPI a chance!
		
	LOG_V("rf233_set_trx_cmd: EXIT\r\n");
}

void rf233_send_message(rf233_message_t * msg)
{
	LOG_V("rf233_send_message: ENTER\r\n");
	
	// Put the message into the frame buffer	
	RESET_CS	
	_send(WRITE_FRAME_BUFFER);	
	_send(msg->PHR);	
	for (int i=0;i<msg->PHR;i++)
		_send(msg->PSDU[i]);	
	SET_CS
	
	// Signal chip to send the message	
	rf233_set_trx_cmd(TRX_CMD_TX_START);
	
	LOG_V("rf233_send_message: EXIT\r\n");
}

//void rf233_get_message(rf233_message_t * msg)
//{
//	LOG_V("rf233_get_message: ENTER\r\n");
//
//
//
//	// Grab the bytes
//	RESET_CS
//
//
//	_send(READ_FRAME_BUFFER);
//
//
//	msg->PHR = _send(0xFF);	// PHR
//	for (int i=0;i<msg->PHR-2;i++)
//		msg->PSDU[i] = _send(0xFF);
//	msg->FCS = ((uint16_t)_send(0xFF)) << 8;
//	msg->FCS |= _send(0xFF);
//	msg->LQI = _send(0xFF);
//	msg->ED = _send(0xFF);
//	msg->RX_STATUS = _send(0xFF);
//
//	SET_CS
//
//	LOG_V("rf233_get_message: EXIT\r\n");
//}


void rf233_get_message(rf233_message_t * msg)
{
	LOG_V("rf233_get_message: ENTER\r\n");

	msg->PHR = _read(0x00);


	for (int i=0;i<msg->PHR-2;i++)
		msg->PSDU[i] = _read(i+1);

	msg->FCS = ((uint16_t)_read(msg->PHR-2)) << 8;
	msg->FCS |= _read(msg->PHR-1);

	msg->LQI = _read(msg->PHR);
	msg->ED = _read(msg->PHR+1);
	msg->RX_STATUS = _read(msg->PHR+2);

	msg->TOM_0 = _read(0x7d);
	msg->TOM_1 = _read(0x7e);
	msg->TOM_2 = _read(0x7f);

	LOG_V("rf233_get_message: EXIT\r\n");

	return;

	msg->FCS = ((uint16_t)_send(0xFF)) << 8;
	msg->FCS |= _send(0xFF);
	msg->LQI = _send(0xFF);
	msg->ED = _send(0xFF);
	msg->RX_STATUS = _send(0xFF);



	msg->PHR = _send(0xFF);	// PHR
	for (int i=0;i<msg->PHR-2;i++)
		msg->PSDU[i] = _send(0xFF);
	msg->FCS = ((uint16_t)_send(0xFF)) << 8;
	msg->FCS |= _send(0xFF);
	msg->LQI = _send(0xFF);
	msg->ED = _send(0xFF);
	msg->RX_STATUS = _send(0xFF);

	SET_CS

	LOG_V("rf233_get_message: EXIT\r\n");
}
