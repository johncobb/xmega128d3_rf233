/*
 * rf233_defs.h
 *
 * Created: 6/26/2012 11:25:03 AM
 *  Author: Eric Rudisill
 */ 


#ifndef RF233_DEFS_H_
#define RF233_DEFS_H_


#define RF233_TRUE			(1)
#define RF233_FALSE			(0)

#define RF233_SPI			SPIC
#define RF233_SPI_PORT		PORTC
#define RF233_SPI_CS		(4)
#define RF233_SPI_MOSI		(5)
#define RF233_SPI_MISO		(6)
#define RF233_SPI_SCK		(7)

#define RF233_AUX_PORT		PORTF
#define RF233_AUX_RST		(0)
#define RF233_AUX_SLP_TR	(1)
#define RF233_AUX_IRQ		(2)
#define RF233_AUX_DIG2		(3)

#define RF233_IRQ_VECTOR	PORTF_INT0_vect

#define RESET(x)			RF233_SPI_PORT.OUTCLR = (1 << x);
#define SET(x)				RF233_SPI_PORT.OUTSET = (1 << x);
#define RESET_CS			RESET(RF233_SPI_CS);
#define SET_CS				SET(RF233_SPI_CS);
#define SET_OUTPUT(x)		RF233_SPI_PORT.DIRSET = (1 << x);
#define SET_INPUT(x)		RF233_SPI_PORT.DIRCLR = (1 << x);

#define RESET_AUX(x)			RF233_AUX_PORT.OUTCLR = (1 << x);
#define SET_AUX(x)				RF233_AUX_PORT.OUTSET = (1 << x);
#define SET_OUTPUT_AUX(x)		RF233_AUX_PORT.DIRSET = (1 << x);
#define SET_INPUT_AUX(x)		RF233_AUX_PORT.DIRCLR = (1 << x);


// SPI PROTOCOL
//
#define READ_REGISTER		(0x80)
#define WRITE_REGISTER		(0xC0)
#define READ_FRAME_BUFFER	(0x20)
#define WRITE_FRAME_BUFFER	(0x60)
#define READ_SRAM			(0x00)
#define WRITE_SRAM			(0x40)

// REGISTERS
//
#define TRX_STATUS			(0x01)
#define TRX_STATE			(0x02)
#define TRX_CTRL_0			(0x03)
#define PHY_RSSI			(0x06)
#define IRQ_MASK			(0x0E)
#define IRQ_STATUS			(0x0F)
#define BATMON				(0x11)

// COMMANDS
//
#define TRX_CMD_NOP				(0x00)
#define TRX_CMD_TX_START		(0x02)
#define TRX_CMD_FORCE_TRX_OFF	(0x03)
#define TRX_CMD_FORCE_PLL_ON	(0x04)
#define TRX_CMD_RX_ON			(0x06)
#define TRX_CMD_TRX_OFF				(0x08)
#define TRX_CMD_PLL_ON			(0x09)
#define TRX_CMD_PREP_DEEP_SLEEP	(0x10)
#define TRX_CMD_RX_AACK_ON		(0x16)
#define TRX_CMD_TX_ARET_ON		(0x19)

// TRX_STATUS
//
#define CCA_DONE_bm			(0x80)
#define CCA_STATUS_bm		(0x40)
#define TRX_STATUS_bm		(0x1F)
#define STATE_TRANSITION_IN_PROGRESS	(0x1F)

// TRX_CTRL_0
//
#define CLKM_SHA_SEL_bm		(0x08)
#define CLKM_CTRL_bm		(0x07)
#define CLKM_SHA_ENABLE		(0x08)
#define CLKM_SHA_DISABLE	(0x00)
#define CLKM_NO_CLOCK		(0x00)
#define CLKM_1MHZ			(0x01)
#define CLKM_2MHZ			(0x02)
#define CLKM_4MHZ			(0x03)
#define CLKM_8MHZ			(0x04)
#define CLKM_16MHZ			(0x05)
#define CLKM_250KHZ			(0x06)
#define CLKM_625KHZ			(0x07)

// PHY_RSSI
//
#define RX_CRC_VALID_bm		(0x80)

// IRQ_MASK
//
#define IRQ_PLL_LOCK_bm		(1 << 0x00)
#define IRQ_PLL_UNLOCK_bm	(1 << 0x01)
#define IRQ_RX_START_bm		(1 << 0x02)
#define IRQ_TRX_END_bm		(1 << 0x03)
#define IRQ_CCA_ED_DONE_bm	(1 << 0x04)
#define IRQ_AWAKE_END_bm    (1 << 0x04)
#define IRQ_AMI_bm			(1 << 0x05)
#define IRQ_TRX_UR_bm		(1 << 0x06)
#define IRQ_BAT_LOW_bm		(1 << 0x07)

#endif /* RF233_DEFS_H_ */