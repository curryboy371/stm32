/*
 * extint.c
 *
 *  Created on: Apr 8, 2025
 *      Author: microsoft
 */


#include "extint.h"
#include "extern.h"
#include "stepmotor.h"

#include "def.h"

#include "elevator.h"

#include <stdio.h>


/**
 * from stm32f4xx_hal_gpio.c
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	switch (GPIO_Pin) {

	case GPIO_PIN_0:
		elevator_set_floor(ELV_1F);
		break;

	case GPIO_PIN_9:
		elevator_set_floor(ELV_2F);
		break;

	case GPIO_PIN_11:
		elevator_set_floor(ELV_3F);
		break;


	case GPIO_PIN_12:
		elevator_set_floor(ELV_4F);
		break;

	default:
		break;

	}


}
