/*
* TestRf233.c
*
* Created: 6/28/2012 3:37:45 PM
*  Author: Eric Rudisill
*/


#include "cph.h"
#include "debug.h"
#include "radio.h"
#include "cli.h"

char autoping = 1;
char brief = 0;
char fast = 0;

void handle_clock_master(void);
void handle_clock_sync(void);


static clock_time_t elapsed = 0;
static volatile uint8_t clock_list_index = 0;
static uint32_t clock_list[10] = {0};

void enable_interrupts(void)
{
	// Turn Interrupts on.
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;
	sei();
}


volatile uint16_t timer_counter = 0;

//void tc_setperiod(void)
//{
//	/* Set period/TOP value. */
//	/* Setup a millisecond timer */
////	TCC1.PER = 16000;
//	/* Select clock source. */
//	TCC1.CTRLA = (TCC1.CTRLA & ~TC1_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
////	TCC1.INTCTRLA = (TCC1.INTCTRLA & ~TC0_OVFINTLVL_gm) | TC_OVFINTLVL_LO_gc;
//
//}

// uint16_t 65535
// 0.0040959375
void tc_setperiod(void)
{
	/* Set period/TOP value. */
	/* Setup a millisecond time
	 *r */
//	TCC1.PER = 16000;
	/* Select clock source. */
	TCC1.CTRLA = (TCC1.CTRLA & ~TC1_CLKSEL_gm ) | TC_CLKSEL_DIV1_gc;
//	TCC1.INTCTRLA = (TCC1.INTCTRLA & ~TC0_OVFINTLVL_gm) | TC_OVFINTLVL_LO_gc;

}


void tc_reset(void)
{
	/* TC must be turned off before a Reset command. */
	TCC1.CTRLA = (TCC1.CTRLA & ~TC1_CLKSEL_gm ) | TC_CLKSEL_OFF_gc;
	/* Issue Reset command. */
	TCC1.CTRLFSET = TC_CMD_RESET_gc;
}

static volatile uint8_t sync_received = 0;
static uint32_t wireless_sync_elapsed = 0;
volatile uint32_t wireless_sync_counter = 0;
static uint32_t wireless_counter_prev = 0;


void receive_cb(radio_message_t * message)
{

	//(6.25*12800)/1000
	wireless_sync_counter = sync_count;

	wireless_sync_elapsed = wireless_sync_counter - wireless_counter_prev;
	wireless_counter_prev = wireless_sync_counter;

	sync_received = 1;




//	if (brief) {
//		printf_P(PSTR("%X "), message->SignalStrengthRaw);
//	}
//	else
//	{
//		printf_P(PSTR("[RCV] MAC:%X SEQ:%X LEN:%X CS:%X SS:%d(%X) DATA: %s\r\n"),
//			message->MacHeader,
//			message->Sequence,
//			message->Length,
//			message->Checksum,
//			message->SignalStrength,
//			message->SignalStrengthRaw,
//			(char*)(message->Data));
//	}
}




/***********************************************************/
/*                                                         */
/*      M A I N                                            */
/*                                                         */
/***********************************************************/
int main(void)
{

	//period = DEFAULT_PERIOD;
	period = DEFAULT_PERIOD_FAST;
	
	// First thing: initialize clock
	clock_init();	
	
	PORTE.DIRSET = PIN0_bm;	//LED
	PORTE.OUTCLR = PIN0_bm;
	PORTE.DIRCLR = PIN4_bm;	//BUTTON
	
	debug_init(debug_cb);
	
	enable_interrupts();
	
	printf_P(PSTR("\r\nRF233 Test 1.0\r\n"));
	printf_P(PSTR("Press any key to start\r\n"));
	printf_P(PSTR("[SND] CLK:%lu \r\n"), clock_millis);



	for(int i=0; i<5; i++) {
		PORTE.OUT = 0x01;
		_delay_ms(50);
		PORTE.OUT = 0x00;
		_delay_ms(50);
	}
	
#if AUTOPING
	autoping = 0xFF;
#else
//	WAIT_FOR_PRESS
#endif
	
	init_sync_timer();
	radio_init();
	
	radio_receive_cb = receive_cb;
	timer = clock_millis;

	// Force a command printout
	debug_in = '?';		


	// Baud divided by 8 bits divided by 1000 millis gives how many bytes per millis
//	115200/8/1000

	//tc_setperiod();
		
	while(1)
	{
		radio_tick();

		handle_input();
		
		if(CLOCK_MASTER == 1) {
			handle_clock_master();
		} else {
			handle_clock_sync();
		}


	}
}

void handle_clock_master(void)
{
	elapsed = clock_millis - timer;

	if (elapsed >= period)
	{

		clock_sync_t *sync = NULL;

		sync->msg_type = 0x01;
		sync->clock = clock_millis;
		sync->temp = 0x01;
		radio_send_clocksync(sync);
		timer = clock_millis;

//		printf_P(PSTR("%lu\r\n"), sync->clock);

	}
}

void handle_clock_sync(void)
{

	if(sync_received == 0)
		return;

	sync_received = 0;

	clock_list[clock_list_index] = wireless_sync_elapsed;
	clock_list_index++;

	if(clock_list_index == 10) {
		clock_list_index = 0;
		printf("\r\n");
		printf_P(PSTR("%lu\r\n"), clock_list[0]);
		printf_P(PSTR("%lu\r\n"), clock_list[1]);
		printf_P(PSTR("%lu\r\n"), clock_list[2]);
		printf_P(PSTR("%lu\r\n"), clock_list[3]);
		printf_P(PSTR("%lu\r\n"), clock_list[4]);
		printf_P(PSTR("%lu\r\n"), clock_list[5]);
		printf_P(PSTR("%lu\r\n"), clock_list[6]);
		printf_P(PSTR("%lu\r\n"), clock_list[7]);
		printf_P(PSTR("%lu\r\n"), clock_list[8]);
		printf_P(PSTR("%lu\r\n"), clock_list[9]);
		printf("\r\n");

	}

}
