/*
 * timer.h
 *
 *  Created on: Mar 31, 2025
 *      Author: microsoft
 */

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"

void delay_us(int us);
void delay_ms(int ms);

uint32_t get_tick();


#endif /* INC_TIMER_H_ */
