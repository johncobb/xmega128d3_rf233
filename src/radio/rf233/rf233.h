/*
 * rf233.h
 *
 * Created: 6/26/2012 11:15:34 AM
 *  Author: Eric Rudisill
 */ 


#ifndef RF233_H_
#define RF233_H_

#define RF233_PSDU_LENGTH	(127)
#define RF233_RSSI_BASE		(-91)

typedef struct {
	uint8_t		PHR;		// length
	uint8_t		PSDU[RF233_PSDU_LENGTH];	// payload
	uint16_t	FCS;		// frame checksum
	uint8_t		LQI;		// link quality indication
	uint8_t		ED;			// energy detection
	uint8_t		RX_STATUS;	// receive status
} rf233_message_t;

typedef void (*rf233_irq_cb_t)(uint8_t);

uint8_t rf233_init_io(void);
uint8_t rf233_init(void);
void rf233_reset(void);
void rf233_status(void);
void rf233_set_clock(uint8_t);
void rf233_set_trx_cmd(uint8_t);
void rf233_send_message(rf233_message_t * msg);
void rf233_get_message(rf233_message_t * msg);

extern rf233_irq_cb_t rf233_irq_cb;

#endif /* RF233_H_ */