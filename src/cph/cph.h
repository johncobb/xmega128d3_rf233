/*
 * cph.h
 *
 *  Created on: Mar 11, 2016
 *      Author: jcobb
 */

#ifndef SRC_CPH_H_
#define SRC_CPH_H_


#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "clock.h"

#define CLOCK_MASTER   0
#define DEFAULT_PERIOD	     (2000)
#define DEFAULT_PERIOD_FAST  (100) //25519 @ 100ms  12928 @ 50ms
#define WAIT_FOR_PRESS	{debug_in = '\0'; while (debug_in == '\0');}




char autoping;
char brief;
char fast;


extern clock_time_t app_timer;
extern clock_time_t app_period;

#endif /* SRC_CPH_H_ */
