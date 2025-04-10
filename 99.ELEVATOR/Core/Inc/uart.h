/*
 * uart.h
 *
 *  Created on: Mar 31, 2025
 *      Author: microsoft
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"

#define COMMAND_NUMBER 20
#define COMMAND_LENGTH 40



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

void pc_command_processing();

void show_command();

#endif /* INC_UART_H_ */
