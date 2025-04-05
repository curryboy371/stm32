/*
 * led.c
 *
 *  Created on: Mar 26, 2025
 *      Author: microsoft
 */


#include "led.h"
#include "def.h"
#include "extern.h"
#include "timer.h"


void led_all_on() {

#if ACCESS_MODE == USE_HAL
	HAL_GPIO_WritePin(GPIOB, LED_PIN_ALL, 1);
#elif ACCESS_MODE == USE_CONST
	*GPIOB_ODR = 0x00ff;
#elif ACCESS_MODE == USE_STRUCT
	GPIOB->ODR = 0x00ff;
#endif


#if ACCESS_MODE == USE_HAL

#elif ACCESS_MODE == USE_CONST

#elif ACCESS_MODE == USE_STRUCT

#endif

}

void led_all_off() {

#if ACCESS_MODE == USE_HAL
	HAL_GPIO_WritePin(GPIOB, LED_PIN_ALL, 0);
#elif ACCESS_MODE == USE_CONST
	*GPIOB_ODR = 0x00;
#elif ACCESS_MODE == USE_STRUCT
	GPIOB->ODR = 0x00;
#endif

}

void shift_left_ledon(void) {

	led_all_off();
#if ACCESS_MODE == USE_HAL
	for(int i = 0; i < 8; ++i) {

		HAL_GPIO_WritePin(GPIOB, 1 << i, 1);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, 1 << i, 0);
	}
#elif ACCESS_MODE == USE_CONST
	for(int i = 0; i < 8; ++i) {
		*GPIOB_ODR = 1 << i;
		HAL_Delay(100);
	}

#elif ACCESS_MODE == USE_STRUCT
	for(int i = 0; i < 8; ++i) {
		GPIOB->ODR = 1 << i;
		HAL_Delay(100);
	}
#endif


}

void shift_right_ledon(void) {

	led_all_off();
#if ACCESS_MODE == USE_HAL
	for(uint16_t i = 0; i < 7; ++i) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7 >> i, 1);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7 >> i, 0);
	}
#elif ACCESS_MODE == USE_CONST
	for(uint16_t i = 0; i < 7; ++i) {
		*GPIOB_ODR = GPIO_PIN_7 >> i;
		HAL_Delay(100);
	}
#elif ACCESS_MODE == USE_STRUCT
	for(uint16_t i = 0; i < 7; ++i) {
		GPIOB->ODR = GPIO_PIN_7 >> i;
		HAL_Delay(100);
	}
#endif


}


void shift_left_keep_led_on(void) {

	led_all_off();
#if ACCESS_MODE == USE_HAL
	for(int i = 0; i < 8; ++i) {
		HAL_GPIO_WritePin(GPIOB, 1 << i, 1);
		HAL_Delay(100);
	}
#elif ACCESS_MODE == USE_CONST
	for(uint16_t i = 0; i < 8; ++i) {

		*GPIOB_ODR |= 1 << i;
		HAL_Delay(100);
	}
#elif ACCESS_MODE == USE_STRUCT
	for(uint16_t i = 0; i < 8; ++i) {
		GPIOB->ODR |= 1 << i;
		HAL_Delay(100);
	}
#endif

}

void shift_right_keep_led_on(void) {


	led_all_off();
#if ACCESS_MODE == USE_HAL
	for(uint16_t i = 0; i < 8; ++i) {

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7 >> i, 1);
		HAL_Delay(100);
	}
#elif ACCESS_MODE == USE_CONST
	for(uint16_t i = 0; i < 8; ++i) {

		*GPIOB_ODR |= GPIO_PIN_7 >> i;
		HAL_Delay(100);
	}
#elif ACCESS_MODE == USE_STRUCT
	for(uint16_t i = 0; i < 8; ++i) {
		GPIOB->ODR |= GPIO_PIN_7 >> i;
		HAL_Delay(100);
	}
#endif



}


void flower_ledon(void) {

	led_all_off();
#if ACCESS_MODE == USE_HAL
	 for(int i = 0 ; i < 4; ++i) {

		HAL_GPIO_WritePin(GPIOB, 0x10 << i | 0x08 >> i, 1);
		HAL_Delay(100);
	 }
#elif ACCESS_MODE == USE_CONST
	 for(int i = 0 ; i < 4; ++i) {
		 //*(unsigned int*)0x040020414 |= 0x10 << i | 0x08 >> i;
		 *GPIOB_ODR |= 0x10 << i | 0x08 >> i;
		 HAL_Delay(100);
	 }
#elif ACCESS_MODE == USE_STRUCT
	 for(int i = 0 ; i < 4; ++i) {
		 //*(unsigned int*)0x040020414 |= 0x10 << i | 0x08 >> i;
		 GPIOB->ODR |= 0x10 << i | 0x08 >> i;
		 HAL_Delay(100);
	 }
#endif



}
void flower_ledoff(void) {

	 led_all_on();

#if ACCESS_MODE == USE_HAL

	 for(int i = 0 ; i < 4; ++i) {
		HAL_GPIO_WritePin(GPIOB, 0b10000000 >> i | 0b00000001 << i, 0);
		HAL_Delay(100);
	 }

#elif ACCESS_MODE == USE_CONST

	 for(int i = 0 ; i < 4; ++i) {
		*GPIOB_ODR &= (~(0b10000000 >> i) & 0xf0) | (~(0b00000001 << i) & 0x0f);
		HAL_Delay(100);
	 }

#elif ACCESS_MODE == USE_STRUCT

	 for(int i = 0 ; i < 4; ++i) {
		GPIOB->ODR &= (~(0b10000000 >> i) & 0xf0) | (~(0b00000001 << i) & 0x0f);
		HAL_Delay(100);
	 }
#endif

}

void led_main() {

	uint8_t led_buff[8] = {0xFF, 0x0F, 0xF0, 0x00,0xFF, 0x0F, 0xF0, 0x00};

	while(1) {

#if 1

		//uint8_t temp = 0b11010001;
		//HAL_SPI_Transmit(&hspi2,led_buff, 1, 1);
		GPIOB->ODR &= ~GPIO_PIN_13; // latch핀을 pull-down ODR(Output Data Register)
		GPIOB->ODR |= GPIO_PIN_13; // latch핀을 pull-up ODR(Output Data Register)
		HAL_Delay(2000);
		//HAL_SPI_Transmit(&hspi2, &led_buff[3], 1, 1);
		GPIOB->ODR &= ~ GPIO_PIN_13;
		GPIOB->ODR |= GPIO_PIN_13;
		HAL_Delay(2000);
#else
		for (int i=0; i < 4; i++)
		{
			HAL_SPI_Transmit(&hspi2, &led_buff[i], 1, 1);
			GPIOB->ODR &= ~ GPIO_PIN_13; // latch핀을 pull-down
			GPIOB->ODR |= GPIO_PIN_13; // // latch핀을 pull-up
			HAL_Delay(1000);
		}
#endif

	}


}
