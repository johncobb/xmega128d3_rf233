/*
 * radio_state.h
 *
 * Created: 7/6/2012 4:28:00 PM
 *  Author: Eric Rudisill
 */ 


#ifndef RADIO_STATE_H_
#define RADIO_STATE_H_


// State management
typedef void (*state_func)(void);
typedef void (*state_func_irq)(uint8_t);

typedef enum
{
	INITIALIZING = 0,
	LISTENING,
	SLEEPING,
	REPORTING,
	SYNCHING,
	REPEATING,
	DEBUGGING
} STATES;

typedef struct
{
	STATES current_state;
	state_func enter;
	state_func tick;
	state_func_irq interrupt;
	state_func exit;
} state_table_t;

void radio_set_state(STATES new_state);

#endif /* RADIO_STATE_H_ */
