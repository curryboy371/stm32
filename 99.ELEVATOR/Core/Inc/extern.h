/*
 * extern.h
 *
 *  Created on: Mar 26, 2025
 *      Author: microsoft
 */

#ifndef INC_EXTERN_H_
#define INC_EXTERN_H_

#include "def.h"

//timer
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;
extern volatile int TIM10_servo_motor_counter;
extern volatile int TIM11_1ms_counter;
extern volatile int TIM11_1ms_delay_counter[MODULE_MAX];
extern volatile int TIM11_line0;
extern volatile int TIM11_line1;

// uart
extern UART_HandleTypeDef huart2;

extern uint8_t rx_data; // uart2 rx byte


// i2c
//extern I2C_HandleTypeDef hi2c1;

// spi
//extern SPI_HandleTypeDef hspi2;

extern t_print_option prt_option;

extern void flash_main();

extern void flash_set_time(uint8_t* pData, uint32_t size);
extern void flash_write_time(uint8_t* pData, uint32_t size);

extern t_setep_motor_state stepmotor_state;

#endif /* INC_EXTERN_H_ */
