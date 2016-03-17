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

extern volatile clock_time_t wireless_sync_elapsed;
extern volatile uint8_t sync_received;
extern volatile clock_time_t wireless_sync_prev;

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



typedef struct{

	uint8_t status;      //initial value of register as ISR is entered
	uint8_t  event;		//event type
//	uint8_t  aatset;		//auto ACK TX bit is set
	uint16_t datalength;	//length of frame
	uint8_t  fctrl[2];	//frame control bytes
//	uint8_t  dblbuff ;	//set if double buffer is enabled

}rf233_callback_data_t;

// -------------------------------------------------------------------------------------------------------------------
// Structure to hold device data
typedef struct
{
    uint32_t      device_id ;
//    uint32      partID ;
//    uint32      lotID ;
//    uint8       chan;               // Added channel here - used in the reading of accumulator
//    uint8       longFrames ;        // Flag in non-standard long frame mode
//    uint8       otprev ;            // OTP revision number (read during initialisation)
//    uint32      txFCTRL ;           // Keep TX_FCTRL register config
//    uint8       xtrim;              // XTAL trim value read from OTP
//    uint8       dblbuffon;          // Double RX buffer mode flag
    uint32_t      sys_cfg_reg ;         // Local copy of system config register
    uint16_t      sleep_mode;         // Used for automatic reloading of LDO tune and microcode at wake-up
    rf233_callback_data_t cdata;      // Callback data structure

    uint8_t       wait4resp ;         // wait4response was set with last TX start command
    int         prf_index ;

    void (*rf233_txcallback)(const rf233_callback_data_t *txd);
    void (*rf233_rxcallback)(const rf233_callback_data_t *rxd);

} rf233_local_data_t ;

static rf233_local_data_t rf233local ; // Static local device data

typedef void (*radio_receive_cb_t)(radio_message_t *);
typedef void (*radio_tof_cb_t)(radio_message_t *);

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
