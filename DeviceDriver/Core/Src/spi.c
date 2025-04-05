/*
 * spi.c
 *
 *  Created on: Apr 3, 2025
 *      Author: microsoft
 */

#include "spi.h"

#include "def.h"


void spi_init() {

	// init

	*GPIOB_ODR &= ~(CLK_74HC595_Pin | LATCH_74HC595_Pin | SER_74HC595_Pin);
	//HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin | LATCH_74HC595_Pin | SER_74HC595_Pin, 0);
	HAL_Delay(10);
}

void spi_clock() {

	*GPIOB_ODR |= (CLK_74HC595_Pin);
	*GPIOB_ODR &= ~(CLK_74HC595_Pin);

	//HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 1);
	//HAL_GPIO_WritePin(GPIOB, CLK_74HC595_Pin, 0);
}

void spi_tx_data(uint8_t inData) {

	for(int i = 7; i >= 0; --i) {

		// msb부터
		if(inData & 1 << i) {

			*GPIOB_ODR |= (SER_74HC595_Pin);
			//HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 1);
		}
		else {

			*GPIOB_ODR &= ~(SER_74HC595_Pin);
			//HAL_GPIO_WritePin(GPIOB, SER_74HC595_Pin, 0);
		}

		// clock
		spi_clock();
	}
}
