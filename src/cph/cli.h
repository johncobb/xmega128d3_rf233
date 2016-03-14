/*
 * cli.h
 *
 *  Created on: Mar 11, 2016
 *      Author: jcobb
 */

#ifndef SRC_CPH_CLI_H_
#define SRC_CPH_CLI_H_


//volatile char debug_in = '\0';

volatile char debug_in;

void debug_cb(uint8_t data);
void handle_input(void);

#endif /* SRC_CPH_CLI_H_ */
