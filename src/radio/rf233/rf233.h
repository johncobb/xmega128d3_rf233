/*
 * rf233.h
 *
 * Created: 6/26/2012 11:15:34 AM
 *  Author: Eric Rudisill
 */ 


#ifndef RF233_H_
#define RF233_H_

#define RF233_PSDU_LENGTH	(128)
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
uint8_t rf233_enable_tom(void);


void rf233_reset(void);
void rf233_status(void);
void rf233_set_clock(uint8_t);
void rf233_set_trx_cmd(uint8_t);
void rf233_send_message(rf233_message_t * msg);
void rf233_get_message(rf233_message_t * msg);
void rf233_frame_read(uint8_t *data, uint8_t len);


void rf233_interface_init(void);
void rf233_trx_reg_write(uint8_t addr, uint8_t data);
uint8_t rf233_trx_reg_read(uint8_t addr);
void rf233_trx_frame_read(uint8_t *data, uint8_t length);
void rf233l_trx_frame_write(uint8_t *data, uint8_t length);
uint8_t rf233_trx_bit_read(uint8_t addr, uint8_t mask, uint8_t pos);
void rf233_trx_bit_write(uint8_t reg_addr, uint8_t mask, uint8_t pos, uint8_t new_value);
void rf233_trx_sram_write(uint8_t addr, uint8_t *data, uint8_t length);
void rf233_trx_sram_read(uint8_t addr, uint8_t *data, uint8_t length);
uint8_t rf233_spi_read_register_bit(uint8_t addr, uint8_t mask, uint8_t pos);
void rf233_spi_write_register_bit(uint8_t reg_addr, uint8_t mask, uint8_t pos, uint8_t new_value);
extern rf233_irq_cb_t rf233_irq_cb;

#endif /* RF233_H_ */
